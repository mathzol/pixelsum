#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <jpeglib.h>
#include <string.h>


FILE *infile, *outfile;

GtkWidget *window;
GtkWidget *entry1;
GtkWidget *entry2;

#define PIXELS_PER_INCH 1
#define PIXELS_PER_CM 2
#define JPEG 1
#define RAW 2
#define INCH 2.54


/* we will be using this uninitialized pointer later to store raw, uncompressd image */
unsigned char *raw_image = NULL;
char* tmp;
char* name;
char* sdata;

/* dimensions of the image we want to write */
int width;
int height;
int bytes_per_pixel;   /* or 1 for GRACYSCALE images */
int color_space = JCS_RGB; /* or JCS_GRAYSCALE for grayscale images */
int sens;
int kapcs2 = 1;
int kapcs = 1;
float pix;
float tt, ot = 0;

static void check1_toggle_callback( GtkWidget *widget, gpointer   data ) {
       kapcs = PIXELS_PER_INCH;

}

static void check2_toggle_callback( GtkWidget *widget, gpointer   data ) {
       kapcs = PIXELS_PER_CM;
}

static void check3_toggle_callback( GtkWidget *widget, gpointer   data ) {
       kapcs2 = JPEG;
}

static void check4_toggle_callback( GtkWidget *widget, gpointer   data ) {
       kapcs2 = RAW;
}

int raw_analyse() {

	int i, s = 0;
	int c;

    rewind(infile);
	rewind(outfile);
	
    if ( !infile )
	{
		//printf(" A bemeneti fajl nem letezik vagy nem nyithato meg!\n A helyes argomentum sorrend: \n pixelsum.exe <inputfajl> <erzekenyseg> <kapcsolo> <felbontas> <outputfajl> <kapcsolo2>\n Erzekenyseg 255 - 0 ig. (255 = feher szin, 0 = fekete szin)\n Kapcsolo: \"1\"  pix/cm\n           \"2\" pix/inch\n Felbontas vegeredmenyben pixel/cm\n Kapcsolo 2.: \"1\"  JPEG kep\n              \"2\"  RAW Kep (Csak egyenlo oldalu!)");
		return -1;
	}  


	
	if ( !outfile )
	{
		//printf(" Nem adtal meg kimeneti fajlt!\n A helyes argomentum sorrend: \n pixelsum.exe <inputfajl> <erzekenyseg> <kapcsolo> <felbontas> <outputfajl> <kapcsolo2>\n Erzekenyseg 255 - 0 ig. (255 = feher szin, 0 = fekete szin)\n Kapcsolo: \"1\"  pix/cm\n           \"2\" pix/inch\n Felbontas vegeredmenyben pixel/cm\n Kapcsolo 2.: \"1\"  JPEG kep\n              \"2\"  RAW Kep (Csak egyenlo oldalu!)");
		return -1;
	} 

while((c = fgetc(infile)) != EOF) { // addig olvassuk a fájlt amíg végéhez nem érünk
		s = c + s; // össze adjuk az RGB szín értékeket
		if(i < 2) {					// 3x --||--
			i++; }
		else {
			if(s > 3*sens) {		// ha színek értéke nagyobb 3*sens nél akkor fehérre javítjuk
				c = 255;
				for(i = 0; i < 3; i++) {
					fputc(c, outfile);
					}
					s = 0;
					i = 0;
					ot = ot + 1.0;}
			else {					// ha nem akkor feketére
				c = 0;
				for(i = 0; i < 3; i++) {	
					fputc(c, outfile); }
					s = 0;
					i = 0;
					tt = tt + 1.0; }
		}
	}	

/*	printf(" Kep adatai (.RAW):\n");

	printf("  Kornyezet: %.0f pix\n  Targy: %.0f pix \n", ot, tt);
	ot = ot + tt; // fehér + fekete pixelek száma = az összesével
	printf("  Felbontas: %.3f pix/cm\n", pix); // felbontás aránya
	printf("  Erzekenyseg: %d \n", sens);
	tt = tt / (pix * pix); // felbontás négyzetével (1 cm²) osztjuk a fekete pixelek számát  

	printf("  A targy terulete: %.2f cm² \n  Ossz terulet: %.2f cm² \n  Pixelek szama: %.0f pix\n", tt, ot / (pix * pix), ot);
	printf("  Meret: %.0f x %.0f pix", sqrt(ot), sqrt(ot)); */
	
	tmp = (char*)malloc(500);
	sprintf(tmp, "\n Fájlnév: %s \n\n Tárgy területe = %0.2f cm²\n\n Környezet = %.0f pix\n Tárgy = %0.0f pix\n Felbontás = %.3f pix/cm\n Érzékenység = %d\n Össz terület = %.2f cm²\n Méret = %.0f pix X %.0f pix\n Pixelek száma = %.0f pix", name, tt/(pix*pix), ot, tt, pix, sens, (ot+tt)/(pix*pix), sqrt(ot+tt), sqrt(ot+tt), ot+tt);
	
	sdata = (char*)malloc(500);
	sprintf(sdata,"Fájlnév:\t%s\tTárgy területe(cm²)=\t%0.2f\tKörnyezet(pix)=\t%.0f\tTárgy(pix)=\t%0.0f\tFelbontás(pix/cm)=\t%.3f\tÉrzékenység=\t%d\tÖssz terület(cm²)=\t%.2f\tMéret(pix)=\t%d x %d\tPixelek száma(cm²)=\t%.0f\n", name, tt/(pix*pix), ot, tt, pix, sens, (ot+tt)/(pix*pix), sqrt(ot+tt), sqrt(ot+tt), ot+tt); 
	
	fclose(outfile);
	return 1;
}

int read_jpeg_file() {
    
	/* these are standard libjpeg structures for reading(decompression) */
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;
	/* libjpeg data structure for storing one row, that is, scanline of an image */
	JSAMPROW row_pointer[1];
	
	unsigned long location = 0;
	int i, d, a, k, h = 0;
	
	if ( !infile )
	{
		//printf(" A bemeneti fajl nem letezik vagy nem nyithato meg!\n A helyes argomentum sorrend: \n pixelsum.exe <inputfajl> <erzekenyseg> <kapcsolo> <felbontas> <outputfajl> <kapcsolo2>\n Erzekenyseg 255 - 0 ig. (255 = feher szin, 0 = fekete szin)\n Kapcsolo: \"1\"  pix/cm\n           \"2\" pix/inch\n Felbontas vegeredmenyben pixel/cm\n Kapcsolo 2.: \"1\"  JPEG kep\n              \"2\"  RAW Kep (Csak egyenlo oldalu!)");
		return -1;
	}
    
    rewind(infile);
     
	/* here we set up the standard libjpeg error handler */
	cinfo.err = jpeg_std_error( &jerr );
	/* setup decompression process and source, then read JPEG header */
	jpeg_create_decompress( &cinfo );
	/* this makes the library read from infile */
	jpeg_stdio_src( &cinfo, infile );
	/* reading the image header which contains image information */
	jpeg_read_header( &cinfo, TRUE );
	/* Uncomment the following to output image information, if needed. */
	
	width = cinfo.image_width;
	height = cinfo.image_height;
	bytes_per_pixel = cinfo.num_components;
	// color_space = cinfo.jpeg_color_space;
	
//	printf( " Kep adatai (.JPEG):\n" );
//	printf( "Color components per pixel: %d.\n", cinfo.num_components );
//	printf( "Color space: %d.\n", cinfo.jpeg_color_space );
	
	/* Start decompression jpeg here */
	jpeg_start_decompress( &cinfo );

	/* allocate memory to hold the uncompressed image */
	raw_image = (unsigned char*)malloc( cinfo.output_width*cinfo.output_height*cinfo.num_components );
	/* now actually read the jpeg into the raw buffer */
	row_pointer[0] = (unsigned char *)malloc( cinfo.output_width*cinfo.num_components );
	/* read one scan line at a time */

	while( cinfo.output_scanline < cinfo.image_height )
	{
		jpeg_read_scanlines( &cinfo, row_pointer, 1 );
		for( i=0; i<cinfo.image_width*cinfo.num_components;i++) {
			if(h > 2) {
				if(d >= 3*sens) {
					for(a = 0; a < 3; a++){
						raw_image[location++] = 255; }
					d = 0;
					h = 0;
					ot = ot + 1.0;}
				else {
					d = 0;
					for(a = 0; a < 3; a++){
						raw_image[location++] = 0;}
					h = 0;
					tt = tt + 1.0;}
			}
				k = row_pointer[0][i];
				d = k + d;
				h++;
	} 	
} 

	
/*	printf("  Kornyezet: %.0f pix\n  Targy: %.0f pix \n", ot, tt);
	ot = ot + tt; // fehér + fekete pixelek száma = az összesével
	printf("  Felbontas: %.3f pix/cm\n", pix); // felbontás aránya
	printf("  Erzekenyseg: %d \n", sens);
	tt = tt / (pix * pix); // felbontás négyzetével (1 cm²) osztjuk a fekete pixelek számát  

	printf("  A targy terulete: %.2f cm² \n  Ossz terulet: %.2f cm² \n  Pixelek szama: %.0f pix\n", tt, ot / (pix * pix), ot);
	printf("  Kep merete: %d pixel x %d pixel\n", cinfo.image_width, cinfo.image_height );
//	printf("  Meret: %.0f x %.0f pix", sqrt(ot), sqrt(ot)); */
	tmp = (char*)malloc(500);
	sprintf(tmp, "\n Fájlnév: %s \n\n Tárgy területe = %0.2f cm²\n\n Környezet = %.0f pix\n Tárgy = %0.0f pix\n Felbontás = %.3f pix/cm\n Érzékenység = %d\n Össz terület = %.2f cm²\n Méret = %d x %d pix\n Pixelek száma = %.0f pix", name, tt/(pix*pix), ot, tt, pix, sens, (ot+tt)/(pix*pix), cinfo.image_width, cinfo.image_height, ot+tt);
	
	sdata = (char*)malloc(500);
	sprintf(sdata,"Fájlnév:\t%s\tTárgy területe(cm²)=\t%0.2f\tKörnyezet(pix)=\t%.0f\tTárgy(pix)=\t%0.0f\tFelbontás(pix/cm)=\t%.3f\tÉrzékenység=\t%d\tÖssz terület(cm²)=\t%.2f\tMéret(pix)=\t%d x %d\tPixelek száma(cm²)=\t%.0f\n", name, tt/(pix*pix), ot, tt, pix, sens, (ot+tt)/(pix*pix), cinfo.image_width, cinfo.image_height, ot+tt); 
	
	/* wrap up decompression, destroy objects, free pointers and close open files */
	jpeg_finish_decompress( &cinfo );
	jpeg_destroy_decompress( &cinfo );
	free( row_pointer[0] );
	//fclose( infile );
	/* yup, we succeeded! */
	return 1;
}

int write_jpeg_file() {
	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr jerr;
	
	/* this is a pointer to one row of image data */
	JSAMPROW row_pointer[1];	
	
	if ( !outfile )
	{
		//printf(" Nem adtal meg kimeneti fajlt!\n A helyes argomentum sorrend: \n pixelsum.exe <inputfajl> <erzekenyseg> <kapcsolo> <felbontas> <outputfajl> <kapcsolo2>\n Erzekenyseg 255 - 0 ig. (255 = feher szin, 0 = fekete szin)\n Kapcsolo: \"1\"  pix/cm\n           \"2\" pix/inch\n Felbontas vegeredmenyben pixel/cm\n Kapcsolo 2.: \"1\"  JPEG kep\n              \"2\"  RAW Kep (Csak egyenlo oldalu!)");
		return -1;
	} 
	
	rewind(outfile);
	
	cinfo.err = jpeg_std_error( &jerr );
	jpeg_create_compress(&cinfo);
	jpeg_stdio_dest(&cinfo, outfile);

	/* Setting the parameters of the output file here */
	cinfo.image_width = width;	
	cinfo.image_height = height;
	cinfo.input_components = bytes_per_pixel;
	cinfo.in_color_space = color_space;
    /* default compression parameters, we shouldn't be worried about these */
	jpeg_set_defaults( &cinfo );
	/* Now do the compression .. */
	jpeg_start_compress( &cinfo, TRUE );
	/* like reading a file, this time write one row at a time */
	while( cinfo.next_scanline < cinfo.image_height )
	{
		row_pointer[0] = &raw_image[ cinfo.next_scanline * cinfo.image_width *  cinfo.input_components];
		jpeg_write_scanlines( &cinfo, row_pointer, 1 );
	}
	/* similar to read file, clean up after we're done compressing */
	jpeg_finish_compress( &cinfo );
	jpeg_destroy_compress( &cinfo );
	fclose( outfile );
	/* success code is 1! */
	return 1;
}

int save_r = 1;

int save_data(void) {

	FILE *outdata;
	
	outdata = fopen("meresei_adatok.txt" , "a");
	
	if(!outdata){
		save_r = -1; return -1;}
	else {
		save_r = 1; }
	
	fprintf(outdata, "%s", sdata);
	
	fclose(outdata);
	return 1;
}

static void save_data_ok () {
   GtkWidget *dialog, *label, *content_area, *fixed, *ok, *warning;
   /* Create the widgets */
   dialog = gtk_dialog_new_with_buttons ("Adatok tárolása",
                                         GTK_WINDOW(window),
                                         GTK_DIALOG_DESTROY_WITH_PARENT,
                                         GTK_STOCK_OK,
                                         GTK_RESPONSE_NONE,
                                         NULL);
   gtk_window_set_resizable(GTK_WINDOW(dialog), FALSE);
   content_area = gtk_dialog_get_content_area (GTK_DIALOG (dialog));
   
   fixed = gtk_fixed_new();
   
   if(save_r == 1) { 
      gtk_widget_set_size_request(dialog, 280, 140);  
      ok = gtk_image_new_from_file("share/images/ok.png");
      gtk_fixed_put(GTK_FIXED(fixed), ok, 10, 5);       
      label = gtk_label_new ("Az adatok tárolása megtörtént!");
      gtk_fixed_put(GTK_FIXED(fixed), label, 100, 35);}
   else {
      gtk_widget_set_size_request(dialog, 280, 130);  
      warning = gtk_image_new_from_file("share/images/warning.png");
      gtk_fixed_put(GTK_FIXED(fixed), warning, 10, 5);
      label = gtk_label_new ("Az adatok tárosála sikertelen!\nKérem ellenőrizze,\nhogy van-e joga a lemezre írni\n vagy a fájl nem-e írásvédett!");
      gtk_fixed_put(GTK_FIXED(fixed), label, 100, 15); }
	  
   /* Ensure that the dialog box is destroyed when the user responds. */
   g_signal_connect_swapped (dialog,
                             "response",
                             G_CALLBACK (gtk_widget_destroy),
                             dialog);
   /* Add the label, and show everything we've added to the dialog. */
    
   gtk_container_add (GTK_CONTAINER (content_area), fixed);
   gtk_widget_show_all (dialog);
   gtk_dialog_run(GTK_DIALOG(dialog));
}

void eredmeny_ok(GtkWidget *widget, gpointer window) {
   GtkWidget *dialog, *label, *content_area, *sumimage, *fixed, *warning, *save;
   /* Create the widgets */
    dialog = gtk_dialog_new_with_buttons ("Eredmény",
                                         GTK_WINDOW(window),
                                         GTK_DIALOG_DESTROY_WITH_PARENT,
                                         GTK_STOCK_OK,
                                         GTK_RESPONSE_NONE,
                                         NULL);
	gtk_window_set_resizable(GTK_WINDOW(dialog), FALSE);
	gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER);
	content_area = gtk_dialog_get_content_area (GTK_DIALOG (dialog));
   
   fixed = gtk_fixed_new();
   
   if(!infile) { 
	  gtk_widget_set_size_request(dialog, 280, 130);  
      warning = gtk_image_new_from_file("share/images/warning.png");
      gtk_fixed_put(GTK_FIXED(fixed), warning, 10, 5);       
      label = gtk_label_new ("Még nem adtál meg bemeneti fájlt!");
      gtk_fixed_put(GTK_FIXED(fixed), label, 100, 35);
	  g_signal_connect_swapped (dialog,
                             "response",
                             G_CALLBACK (gtk_widget_destroy),
                             dialog);
      gtk_container_add (GTK_CONTAINER (content_area), fixed);
      gtk_widget_show_all (dialog);
	  gtk_dialog_run(GTK_DIALOG(dialog)); }
   else if(!outfile) {
      gtk_widget_set_size_request(dialog, 280, 130);  
      warning = gtk_image_new_from_file("share/images/warning.png");
      gtk_fixed_put(GTK_FIXED(fixed), warning, 10, 5);
      label = gtk_label_new ("Még nem adtál meg kimeneti fájlt!");
      gtk_fixed_put(GTK_FIXED(fixed), label, 100, 35);
	  g_signal_connect_swapped (dialog,
                             "response",
                             G_CALLBACK (gtk_widget_destroy),
                             dialog);
      gtk_container_add (GTK_CONTAINER (content_area), fixed);
      gtk_widget_show_all (dialog);
	  gtk_dialog_run(GTK_DIALOG(dialog));}         
   else {
      gtk_widget_set_size_request(dialog, 350, 220);  
      sumimage = gtk_image_new_from_file("share/images/sum.png");
      gtk_fixed_put(GTK_FIXED(fixed), sumimage, 10, 30); 
      label = gtk_label_new (tmp);
      gtk_fixed_put(GTK_FIXED(fixed), label, 130, 0);
	  save = gtk_button_new_with_label("Tárolás");
	  gtk_fixed_put(GTK_FIXED(fixed), save, 10, 180);
	  gtk_widget_set_size_request(save, 80, 30);
	  g_signal_connect (G_OBJECT (save), "clicked", G_CALLBACK (save_data), NULL);
	  g_signal_connect (G_OBJECT (save), "clicked", G_CALLBACK (save_data_ok), NULL);
      g_signal_connect_swapped (dialog,
                             "response",
                             G_CALLBACK (gtk_widget_destroy),
                             dialog);
      gtk_container_add (GTK_CONTAINER (content_area), fixed);
      gtk_widget_show_all (dialog);
	}

}

int sum_process() {
    
    sens = 0;
    pix = 0;
    ot = 0;
    tt = 0;
                  	   
		sens = gtk_spin_button_get_value_as_int( GTK_SPIN_BUTTON(entry2) );
		pix = gtk_spin_button_get_value( GTK_SPIN_BUTTON( entry1 )  ); 
		   			  
    if(kapcs == 1) {
            pix = pix / INCH; }                      
                          			   
	if(kapcs2 == 1) {
		/* Try opening a jpeg */
		if( read_jpeg_file() > 0 ) 
		{
			/* then copy it to another file */ 
			if( write_jpeg_file() < 0 ) return -1;
		}
		else return -1;
		return 0; }
	if(kapcs2 == 2) { 
		if(raw_analyse() < 0 ) {
			return 0; } }
			
    
}

int fileclose = 0;

static void input_button_callback( GtkWidget *widget, gpointer   data ) {
GtkWidget *dialog;
dialog = gtk_file_chooser_dialog_new ("Fájl megnyitása",
				      GTK_WINDOW(window),
				      GTK_FILE_CHOOSER_ACTION_OPEN,
				      GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				      GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
				      NULL);
if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
  {
    char *filename;
    filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
	infile = fopen(filename, "rb");
	fileclose = 1;
	name = (char*)malloc(100);
	sprintf(name,"%s", strrchr(filename,'\\')+1);
    g_free (filename);
  }
gtk_widget_destroy (dialog);

}

static void output_button_callback( GtkWidget *widget, gpointer   data ) {
GtkWidget *dialog;
dialog = gtk_file_chooser_dialog_new ("Fájl mentése",
				      GTK_WINDOW(window),
				      GTK_FILE_CHOOSER_ACTION_SAVE,
				      GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				      GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
				      NULL);

if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
  {
    char *filename;
    filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
	outfile = fopen(filename, "wb");
    g_free (filename);
  }
gtk_widget_destroy (dialog);

}

void show_about(GtkWidget *widget, gpointer data) {

  GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file("share/images/info.png", NULL);

  GtkWidget *dialog = gtk_about_dialog_new();
  gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER);
  gtk_about_dialog_set_name(GTK_ABOUT_DIALOG(dialog), "PixelSum");
  gtk_about_dialog_set_version(GTK_ABOUT_DIALOG(dialog), "1.3"); 
  gtk_about_dialog_set_copyright(GTK_ABOUT_DIALOG(dialog), 
      "(c) Máthé Zoltán\n Elérhetöség: mathzoltan@gmail.com");
  gtk_about_dialog_set_comments(GTK_ABOUT_DIALOG(dialog), 
     "A program egy skennelt tárgy területet számitja ki.\n\n Programnyelv: \"C\"\n Forditó: GCC\n Grafikus felület: GTK+ ver. 2.16.4-20090708\n");
/*  gtk_about_dialog_set_website(GTK_ABOUT_DIALOG(dialog), 
      "http://www.batteryhq.net"); */
  gtk_about_dialog_set_logo(GTK_ABOUT_DIALOG(dialog), pixbuf);
  g_object_unref(pixbuf), pixbuf = NULL;
  gtk_dialog_run(GTK_DIALOG (dialog));
  gtk_widget_destroy(dialog);

}

GdkPixbuf *create_pixbuf(const gchar * filename) {
   GdkPixbuf *pixbuf;
   GError *error = NULL;
   pixbuf = gdk_pixbuf_new_from_file(filename, &error);
   if(!pixbuf) {
      fprintf(stderr, "%s\n", error->message);
      g_error_free(error);
   }

   return pixbuf;
}

void show_question(GtkWidget *widget, gpointer window) {
  GtkWidget *dialog;
  
  dialog = gtk_message_dialog_new(window,
            GTK_DIALOG_DESTROY_WITH_PARENT,
            GTK_MESSAGE_QUESTION,
            GTK_BUTTONS_YES_NO,
            "Biztos hogy kilép?");
            
  gtk_window_set_title(GTK_WINDOW(dialog), "Kérdés");
  gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_YES);
                                            
  if (gtk_dialog_run (GTK_DIALOG (dialog)) != GTK_RESPONSE_YES) {
     gtk_widget_destroy(dialog);}
  else {
	 if(fileclose == 1){
	 fclose( infile );
	 fclose( outfile ); }
     gtk_widget_destroy(dialog);
     gtk_widget_destroy(window); }  
}

int main( int argc, char *argv[]) {
  GtkWidget *fixed;
  
  GtkWidget *exit;
  GtkWidget *sum;
  GtkWidget *input_button;
  GtkWidget *output_button;
  GtkWidget *about;

  GtkWidget *check1;
  GtkWidget *check2;
  GtkWidget *check3;
  GtkWidget *check4;
  
  GtkWidget *label;
  GtkWidget *label2;
  GtkWidget *label3;
  GtkWidget *label4;
  GtkWidget *label5;
  GtkWidget *label6;
  GtkWidget *label7;
  
  GtkWidget *scale;
  GtkWidget *logo;
  
  GtkAdjustment *integer, *float_pt;

  gtk_init(&argc, &argv);

  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(window), "PixelSum 1.3 GUI");
  gtk_widget_set_size_request(window, 680, 505);
  gtk_window_set_icon(GTK_WINDOW(window), create_pixbuf("share/images/PixelSumIcon.png"));
  gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
  gtk_container_set_border_width(GTK_CONTAINER(window), 8);
  gtk_window_set_resizable(GTK_WINDOW(window), FALSE);

  fixed = gtk_fixed_new();
  gtk_container_add(GTK_CONTAINER(window), fixed);
  
  sum = gtk_button_new_with_label("Eredmény");
  gtk_fixed_put(GTK_FIXED(fixed), sum, 280, 160);
  gtk_widget_set_size_request(sum, 80, 30);
  g_signal_connect (G_OBJECT (sum), "clicked", G_CALLBACK (sum_process), NULL);
  g_signal_connect (G_OBJECT (sum), "clicked", G_CALLBACK (eredmeny_ok), NULL);
  
  input_button = gtk_button_new_with_label("Megnyitás");
  gtk_fixed_put(GTK_FIXED(fixed), input_button, 0, 20);
  gtk_widget_set_size_request(input_button, 80, 30);
  g_signal_connect (G_OBJECT (input_button), "clicked", G_CALLBACK (input_button_callback), NULL);
  
  output_button = gtk_button_new_with_label("Mentés");
  gtk_fixed_put(GTK_FIXED(fixed), output_button, 0, 160);
  gtk_widget_set_size_request(output_button, 80, 30);
  g_signal_connect (G_OBJECT (output_button), "clicked", G_CALLBACK (output_button_callback), NULL);

  about = gtk_button_new_with_label("Névjegy");
  gtk_fixed_put(GTK_FIXED(fixed), about, 480, 160);
  gtk_widget_set_size_request(about, 80, 30);
  g_signal_connect (G_OBJECT (about), "clicked", G_CALLBACK (show_about), NULL);

  exit = gtk_button_new_with_label("Kilépés");
  gtk_fixed_put(GTK_FIXED(fixed), exit, 580, 160);
  gtk_widget_set_size_request(exit, 80, 30);



  integer = GTK_ADJUSTMENT(gtk_adjustment_new(240, 0, 255, 1, 2, 2));
  float_pt = GTK_ADJUSTMENT(gtk_adjustment_new(400.0, 0, 99999.9, 0.1, 0.5, 0.5));

  entry1 = gtk_spin_button_new(float_pt, 0.1, 1);
  gtk_fixed_put(GTK_FIXED(fixed), entry1, 0, 125);

  entry2 = gtk_spin_button_new(integer, 0.1, 0);
  gtk_fixed_put(GTK_FIXED(fixed), entry2, 0, 75);
  
  
  label = gtk_label_new("Nyissa meg az elemezni kívánt képet!");
  gtk_fixed_put(GTK_FIXED(fixed), label, 0, 0);
  
  label2 = gtk_label_new("A negativ kép mentésének helye!");
  gtk_fixed_put(GTK_FIXED(fixed), label2, 0, 195);

  label3 = gtk_label_new("Felbontás");
  gtk_fixed_put(GTK_FIXED(fixed), label3, 0, 105);
  
  label4 = gtk_label_new("Érzékenység");
  gtk_fixed_put(GTK_FIXED(fixed), label4, 0, 55);
  
  label5 = gtk_label_new("MEGJEGYZÉS!\n Ha újra szeretné számoltatni az elemezni kívánt képet,\n NEM kell újra megnyitnia!\n Elég ha beállítja az új értékeket és megadja újra a kimeneti fájlt!");
  gtk_fixed_put(GTK_FIXED(fixed), label5, 270, 90);
  
  label6 = gtk_label_new("SEGITSÉG\n A program használata a következő:\n  1. Nyissuk meg az elemezni kívánt fájlt, majd állítsuk be a fájl típusát (.JPEG vagy .RAW).\n  2. Állítsuk be a méréshez a kívánt érzékenységet, a következő módon:\n      \"0\" fekete szín a skálán                                                                                                                   \"255\" a fehér szín a skálán");
  gtk_fixed_put(GTK_FIXED(fixed), label6, 0, 230);
  
  label7 = gtk_label_new("        az érzékenységi értékre gondoljunk úgy mint egy pontra, a színskála pedig a számegyenes,\n        azok a színek amik a ponttól, \"255\" felé terjednek, mind felveszik a \"255\" -os értéket, amelyek\n        a ponttól a \"0\" felé terjednek, pedig a \"0\" értéket fogják felvenni, így végeredményül\n        kapunk egy fekete-fehér képet.\n 3. Állítsuk be a felbontást és adjuk meg milyen mértékegységben értjük azt.\n 4. Válasszuk ki a kimenetifájl helyét és mentsük el.\n 5. Nyomjuk meg az \"Eredmény\" gombot a számolás elvégzéséhez.\n\n FIGYELEM!\n Ha végzett a program a számolással, nyissuk meg a kimenetifájlt és végezzünk ellenörzést,\n hogy a számolni kívánt alakzat formája, megegyezik-e a kimenetifájlon látható alakzat formájával.\n Ha nem, szerekesszük vagy allítsunk az érzékenységen!\n Ne felejtsük a szerkesztett képet, bemenetifájlként megnyitni és új kimenetifájlt megadni!(szerkesztes esetén)");
  gtk_fixed_put(GTK_FIXED(fixed), label7, 0, 320);
  
  
  scale = gtk_image_new_from_file("share/images/colorscale.png");
  gtk_fixed_put(GTK_FIXED(fixed), scale, 25, 296);
  
  logo = gtk_image_new_from_file("share/images/logo.png");
  gtk_fixed_put(GTK_FIXED(fixed), logo, 380, 10);

  check1 = gtk_radio_button_new_with_label(NULL, "Pixel/Inch");
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(fixed), TRUE);
  GTK_WIDGET_UNSET_FLAGS(fixed, GTK_CAN_FOCUS);
  gtk_fixed_put(GTK_FIXED(fixed), check1, 90, 115);
  
  check2 = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(check1),"Pixel/Cm");
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(fixed), TRUE);
  GTK_WIDGET_UNSET_FLAGS(fixed, GTK_CAN_FOCUS);
  gtk_fixed_put(GTK_FIXED(fixed), check2, 90, 135);
  
  
  check3 = gtk_radio_button_new_with_label(NULL, ".JPEG");
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(fixed), TRUE);
  GTK_WIDGET_UNSET_FLAGS(fixed, GTK_CAN_FOCUS);
  gtk_fixed_put(GTK_FIXED(fixed), check3, 90, 15);
  
  check4 = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(check3),".RAW (csak egyenlő oldalú!)");
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(fixed), TRUE);
  GTK_WIDGET_UNSET_FLAGS(fixed, GTK_CAN_FOCUS);
  gtk_fixed_put(GTK_FIXED(fixed), check4, 90, 35);
  
  g_signal_connect (G_OBJECT (check1), "toggled", G_CALLBACK (check1_toggle_callback), NULL);
  g_signal_connect (G_OBJECT (check2), "toggled", G_CALLBACK (check2_toggle_callback), NULL);
  g_signal_connect (G_OBJECT (check3), "toggled", G_CALLBACK (check3_toggle_callback), NULL);
  g_signal_connect (G_OBJECT (check4), "toggled", G_CALLBACK (check4_toggle_callback), NULL);   
  g_signal_connect(G_OBJECT(exit), "clicked", G_CALLBACK(show_question), (gpointer) window);   
  g_signal_connect_swapped(G_OBJECT(window), "destroy", G_CALLBACK(gtk_main_quit), G_OBJECT(window));         
          
  gtk_widget_show_all(window);
  gtk_main();
  
  return 0;
}
