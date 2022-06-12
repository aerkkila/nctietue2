#include <nctietue.h>
#include <stdlib.h>
#include <gtk/gtk.h>

static void activate(GtkApplication* app, gpointer user_data) {
    GtkWidget* window = gtk_application_window_new(app);
    gtk_window_present(GTK_WINDOW(window));
}

int nct_view(nct_vset* vset, int varid) {
    GtkApplication* app = gtk_application_new("ncfigure.id", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    int status = g_application_run(G_APPLICATION(app), 0, NULL);
    g_object_unref(app);

    return status;
}

#ifdef GTKTESTI
int main(int argc, char** argv) {
    nct_vset vset;
    int varid, status;
    nct_read_ncfile_gd(&vset, "kuva.nc");
    varid = nct_get_noncoord_varid(&vset);
    status = nct_view(&vset, varid);
    nct_free_vset(&vset);
    return status;
}
#endif
