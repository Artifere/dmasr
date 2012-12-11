#include <gtk/gtk.h>
#include <stdlib.h>
#include "simulateur.h"
State *s;
FILE *file;
GtkWidget *window;
GtkWidget *spin_button_steps;
GtkWidget *grid_state;
GtkWidget *entry_start;
GtkWidget *entry_end;
GtkTreeStore *tree_store_mem;
GtkWidget *tree_view_mem;
void load_program()
{
  rewind(file);
  int pos = 0;
  while(!feof(file))
    {
      unsigned char c1 = fgetc(file);
      unsigned char c2 = fgetc(file);
      if(feof(file))
	break;
      int instr = (c1 << 8) + c2;
      s->mem = write_mem(s->mem, pos, instr);
      pos++;
    }
}
void reset()
{
  s->mem = free_mem(s->mem);
  int i, j;
  for(i = 0; i < 16; i++)
    for(j = 0; j < 2; j++)
      s->reg[i][j] = 0;
  s->PC = 0;
  s->SP = 0x100000;
  s->RA = 0;
  s->SR = 0;
  s->mem = write_mem(s->mem, 12, 24);
  if(file != NULL)
    load_program();
}
void open_file(char *filename)
{
  if(file != NULL)
    fclose(file);
  file = fopen(filename, "r");
  if(file == NULL)
    {
      GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window), GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, "Error opening file: %s\n", filename);
      gtk_dialog_run(GTK_DIALOG(dialog));
      gtk_widget_destroy(GTK_WIDGET(dialog));
    }
  else
    {
      reset(NULL, NULL);
      load_program();
    }
}
void open_callback()
{
  GtkWidget *dialog_open = gtk_file_chooser_dialog_new("open file", GTK_WINDOW(window), GTK_FILE_CHOOSER_ACTION_OPEN, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, NULL);
  if(gtk_dialog_run(GTK_DIALOG(dialog_open)) == GTK_RESPONSE_ACCEPT)
    {
      char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog_open));
      open_file(filename);
      g_free(filename);
    }
  gtk_widget_destroy(dialog_open);
}
void step_callback()
{
  int steps = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spin_button_steps));
  int i;
  for(i = 0; i < steps; i++)
    execute(s);
}
void update_state_callback()
{
  int i, j;
  char val[11];
  for(i = 0; i < 16; i++)
    for(j = 0; j < 2; j++)
      {
	snprintf(val, 11, "0x%08x", s->reg[i][j]);
	gtk_label_set_text(GTK_LABEL(gtk_grid_get_child_at(GTK_GRID(grid_state), 2 * (1 - j) + 1, i)), val);
      }
  snprintf(val, 11, "0x%08x", s->PC);
  gtk_label_set_text(GTK_LABEL(gtk_grid_get_child_at(GTK_GRID(grid_state), 1, 16)), val);
  snprintf(val, 11, "0x%08x", s->SP);
  gtk_label_set_text(GTK_LABEL(gtk_grid_get_child_at(GTK_GRID(grid_state), 3, 16)), val);
  snprintf(val, 11, "0x%08x", s->RA);
  gtk_label_set_text(GTK_LABEL(gtk_grid_get_child_at(GTK_GRID(grid_state), 1, 17)), val);
  snprintf(val, 11, "%c%c%c%c", s->SR & 0x8 ? 'V' : ' ', s->SR & 0x4 ? 'C' : ' ', s->SR & 0x2 ? 'N' : ' ', s->SR & 0x1 ? 'Z' : ' ');
  gtk_label_set_text(GTK_LABEL(gtk_grid_get_child_at(GTK_GRID(grid_state), 3, 17)), val);
}
gboolean update_mem_foreach(GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, gpointer data)
{
  if(!gtk_tree_model_iter_has_child(GTK_TREE_MODEL(tree_store_mem), iter))
    {
      gchar *address_str;
      gtk_tree_model_get(GTK_TREE_MODEL(tree_store_mem), iter, 0, &address_str, -1);
      unsigned long address = strtoul(address_str, NULL, 0);
      int value = read_mem(s->mem, address);
      char value_str[15];
      snprintf(value_str, 15, "0x%04x (%d)", value, value);
      gtk_tree_store_set(tree_store_mem, iter, 1, value_str, -1);
      g_free(address_str);
    }
  return FALSE;
}
void update_mem_callback()
{
  gtk_tree_model_foreach(GTK_TREE_MODEL(tree_store_mem), update_mem_foreach, NULL);
}
void add_mem_callback()
{
  unsigned long start = strtoul(gtk_entry_get_text(GTK_ENTRY(entry_start)), NULL, 0);
  unsigned long end = strtoul(gtk_entry_get_text(GTK_ENTRY(entry_end)), NULL, 0);
  if(end > start)
    {
      char address[52];
      snprintf(address, 52, "0x%08lx (%ld) -> 0x%08lx (%ld)", start, start, end, end);
      GtkTreeIter iter0;
      gtk_tree_store_append(tree_store_mem, &iter0, NULL);
      gtk_tree_store_set(tree_store_mem, &iter0, 0, address, -1);
      unsigned long i;
      for(i = start; i <= end; i++)
	{
	  char address[24];
	  snprintf(address, 24, "0x%08lx (%ld)", i, i);
	  GtkTreeIter iter;
	  gtk_tree_store_append(tree_store_mem, &iter, &iter0);
	  gtk_tree_store_set(tree_store_mem, &iter, 0, address, -1);
	}
    }
  else
    {
      char address[24];
      snprintf(address, 24, "0x%08lx (%ld)", start, start);
      GtkTreeIter iter;
      gtk_tree_store_append(tree_store_mem, &iter, NULL);
      gtk_tree_store_set(tree_store_mem, &iter, 0, address, 1, -1);
    }
}
void remove_mem_callback()
{
  GtkTreeIter iter;
  if(gtk_tree_selection_get_selected(gtk_tree_view_get_selection(GTK_TREE_VIEW(tree_view_mem)), NULL, &iter))
    {
      if(gtk_tree_store_iter_depth(tree_store_mem, &iter) == 1)
	{
	  GtkTreeIter iter_tmp;
	  gtk_tree_model_iter_parent(GTK_TREE_MODEL(tree_store_mem), &iter_tmp, &iter);
	  iter = iter_tmp;
	}
      gtk_tree_store_remove(tree_store_mem, &iter);
    }
}
int main (int argc, char *argv[])
{
  gtk_init(&argc, &argv);
  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
  GtkWidget *grid_general = gtk_grid_new();
  gtk_container_add(GTK_CONTAINER(window), grid_general);

  grid_state = gtk_grid_new();
  gtk_grid_set_column_spacing(GTK_GRID(grid_state), 20);
  gtk_grid_attach(GTK_GRID(grid_general), grid_state, 0, 1, 1, 1);
  int i;
  for(i = 0; i < 16; i++)
    {
      char name[5];
      snprintf(name, 5, "R%dH", i);
      gtk_grid_attach(GTK_GRID(grid_state), gtk_label_new(name), 0, i, 1, 1);
      gtk_grid_attach(GTK_GRID(grid_state), gtk_label_new(NULL), 1, i, 1, 1);
      snprintf(name, 5, "R%dL", i);
      gtk_grid_attach(GTK_GRID(grid_state), gtk_label_new(name), 2, i, 1, 1);
      gtk_grid_attach(GTK_GRID(grid_state), gtk_label_new(NULL), 3, i, 1, 1);
    }
  gtk_grid_attach(GTK_GRID(grid_state), gtk_label_new("PC"), 0, 16, 1, 1);
  gtk_grid_attach(GTK_GRID(grid_state), gtk_label_new(NULL), 1, 16, 1, 1);
  gtk_grid_attach(GTK_GRID(grid_state), gtk_label_new("SP"), 2, 16, 1, 1);
  gtk_grid_attach(GTK_GRID(grid_state), gtk_label_new(NULL), 3, 16, 1, 1);
  gtk_grid_attach(GTK_GRID(grid_state), gtk_label_new("RA"), 0, 17, 1, 1);
  gtk_grid_attach(GTK_GRID(grid_state), gtk_label_new(NULL), 1, 17, 1, 1);
  gtk_grid_attach(GTK_GRID(grid_state), gtk_label_new("SR"), 2, 17, 1, 1);
  gtk_grid_attach(GTK_GRID(grid_state), gtk_label_new(NULL), 3, 17, 1, 1);

  GtkWidget *image_vexpand = gtk_image_new_from_pixbuf(NULL);
  gtk_widget_set_vexpand(GTK_WIDGET(image_vexpand), TRUE);
  gtk_grid_attach(GTK_GRID(grid_general), image_vexpand, 0, 2, 1, 1);

  GtkWidget *grid_mem = gtk_grid_new();
  gtk_grid_attach(GTK_GRID(grid_general), grid_mem, 1, 0, 1, 3);
  GtkWidget *scrolled_window_mem = gtk_scrolled_window_new(NULL, NULL);
  gtk_grid_attach(GTK_GRID(grid_mem), scrolled_window_mem, 0, 1, 5, 1);
  tree_store_mem = gtk_tree_store_new(2, G_TYPE_STRING, G_TYPE_STRING);
  tree_view_mem = gtk_tree_view_new_with_model(GTK_TREE_MODEL(tree_store_mem));
  gtk_widget_set_hexpand(GTK_WIDGET(tree_view_mem), TRUE);
  gtk_widget_set_vexpand(GTK_WIDGET(tree_view_mem), TRUE);
  gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(tree_view_mem), FALSE);
  GtkTreeViewColumn *tree_view_column_address = gtk_tree_view_column_new_with_attributes(NULL, gtk_cell_renderer_text_new(), "text", 0, NULL);
  gtk_tree_view_column_set_expand(tree_view_column_address, TRUE);
  gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view_mem), tree_view_column_address);
  GtkTreeViewColumn *tree_view_column_value = gtk_tree_view_column_new_with_attributes(NULL, gtk_cell_renderer_text_new(), "text", 1, NULL);
  gtk_tree_view_column_set_expand(tree_view_column_value, TRUE);
  gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view_mem), tree_view_column_value);
  gtk_container_add(GTK_CONTAINER(scrolled_window_mem), tree_view_mem);
  entry_start = gtk_entry_new();
  gtk_entry_set_alignment(GTK_ENTRY(entry_start), 0.5);
  gtk_entry_set_placeholder_text(GTK_ENTRY(entry_start), "Start");
  gtk_entry_set_width_chars(GTK_ENTRY(entry_start), 10);
  gtk_grid_attach(GTK_GRID(grid_mem), entry_start, 0, 0, 1, 1);
  entry_end = gtk_entry_new();
  gtk_entry_set_alignment(GTK_ENTRY(entry_end), 0.5);
  gtk_entry_set_placeholder_text(GTK_ENTRY(entry_end), "End");
  gtk_entry_set_width_chars(GTK_ENTRY(entry_end), 10);
  gtk_grid_attach(GTK_GRID(grid_mem), entry_end, 1, 0, 1, 1);
  GtkWidget *button_add = gtk_button_new_with_label("add");
  g_signal_connect(button_add, "clicked", G_CALLBACK(add_mem_callback), NULL);
  g_signal_connect(button_add, "clicked", G_CALLBACK(update_mem_callback), NULL);
  gtk_grid_attach(GTK_GRID(grid_mem), button_add, 2, 0, 1, 1);
  GtkWidget *button_remove = gtk_button_new_with_label("remove");
  g_signal_connect(button_remove, "clicked", G_CALLBACK(remove_mem_callback), NULL);
  gtk_grid_attach(GTK_GRID(grid_mem), button_remove, 3, 0, 1, 1);
  GtkWidget *image_hexpand = gtk_image_new_from_pixbuf(NULL);
  gtk_widget_set_hexpand(GTK_WIDGET(image_hexpand), TRUE);
  gtk_grid_attach(GTK_GRID(grid_mem), image_hexpand, 4, 0, 1, 1);

  GtkWidget *grid_control = gtk_grid_new();
  gtk_grid_set_row_homogeneous(GTK_GRID(grid_control), TRUE);
  gtk_grid_set_column_homogeneous(GTK_GRID(grid_control), TRUE);
  gtk_grid_attach(GTK_GRID(grid_general), grid_control, 0, 0, 1, 1);
  GtkWidget *button_open = gtk_button_new_with_label("open");
  g_signal_connect(button_open, "clicked", G_CALLBACK(open_callback), NULL);
  g_signal_connect(button_open, "clicked", G_CALLBACK(update_state_callback), NULL);
  g_signal_connect(button_open, "clicked", G_CALLBACK(update_mem_callback), NULL);
  gtk_grid_attach(GTK_GRID(grid_control), button_open, 0, 0, 1, 1);
  GtkWidget *button_reset = gtk_button_new_with_label("reset");
  g_signal_connect(button_reset, "clicked", G_CALLBACK(reset), NULL);
  g_signal_connect(button_reset, "clicked", G_CALLBACK(update_state_callback), NULL);
  g_signal_connect(button_reset, "clicked", G_CALLBACK(update_mem_callback), NULL);
  gtk_grid_attach(GTK_GRID(grid_control), button_reset, 1, 0, 1, 1);
  spin_button_steps = gtk_spin_button_new_with_range(1, 1000000, 1);
  gtk_grid_attach(GTK_GRID(grid_control), spin_button_steps, 0, 1, 1, 1);
  GtkWidget *button_step = gtk_button_new_with_label("step");
  g_signal_connect(button_step, "clicked", G_CALLBACK(step_callback), NULL);
  g_signal_connect(button_step, "clicked", G_CALLBACK(update_state_callback), NULL);
  g_signal_connect(button_step, "clicked", G_CALLBACK(update_mem_callback), NULL);
  gtk_grid_attach(GTK_GRID(grid_control), button_step, 1, 1, 1, 1);

  gtk_widget_show_all(window);

  s = malloc(sizeof(State));
  if(s == NULL)
    {
      printf("error while allocating memory !\n");
      exit(1);
    }
  s->mem = NULL;
  reset(NULL, NULL);
  update_state_callback(NULL, NULL);
  update_mem_callback(NULL, NULL);
  FILE *file = NULL;
  if(argc > 1)
    open_file(argv[1]);
  gtk_main();
  free(s);
  if(file != NULL)
    fclose(file);
  return 0;
}
