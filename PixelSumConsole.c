#include <stdio.h>
#include <jpeglib.h>
#include <stdlib.h>
#define INCH 2.54

int getstr(char *argument, char *string) {

   if(argument[0] == '-' && argument[1] == string[0]) {
      return 1; }
   return 0;
}

/* we will be using this uninitialized pointer later to store raw, uncompressd image */
unsigned char *raw_image = NULL;

/**
 * read_jpeg_file Reads from a jpeg file on disk specified by filename and saves into the 
 * raw_image buffer in an uncompressed format.
 * 
 * \returns positive integer if successful, -1 otherwise
 * \param *filename char string specifying the file name to read from
 *
 */

/* dimensions of the image we want to write */
int width;
int height;
int bytes_per_pixel;   /* or 1 for GRACYSCALE images */
int color_space = JCS_RGB; /* or JCS_GRAYSCALE for grayscale images */
int sens;
int kapcs, kapcs2;
float pix;
float tt, ot = 0;
 
int read_jpeg_file( char *filename ) {
    
	/* these are standard libjpeg structures for reading(decompression) */
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;
	/* libjpeg data structure for storing one row, that is, scanline of an image */
	JSAMPROW row_pointer[1];
	
	FILE *infile = fopen( filename, "rb" );
	unsigned long location = 0;
	int i, d, a, k, h= 0;
	
	if ( !infile )
	{
		printf(" A bemeneti fajl nem letezik vagy nem nyithato meg!\n A helyes argomentum sorrend: \n PixelSumConsole.exe <inputfajl> <erzekenyseg> <kapcsolo> <felbontas> <outputfajl> <kapcsolo2>\n Erzekenyseg 255 - 0 ig. (255 = feher szin, 0 = fekete szin)\n Kapcsolo: \"1\"  pix/cm\n           \"2\" pix/inch\n Felbontas vegeredmenyben pixel/cm\n Kapcsolo 2.: \"1\"  JPEG kep\n              \"2\"  RAW Kep (Csak egyenlo oldalu!)");
		return -1;
	}
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
	
	printf( " Kep adatai (.JPEG):\n" );
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

	
	printf("  Kornyezet: %.0f pix\n  Targy: %.0f pix \n", ot, tt);
	ot = ot + tt; // fehér + fekete pixelek száma = az összesével
	printf("  Felbontas: %.3f pix/cm\n", pix); // felbontás aránya
	printf("  Erzekenyseg: %d \n", sens);
	tt = tt / (pix * pix); // felbontás négyzetével (1 cm²) osztjuk a fekete pixelek számát  

	printf("  A targy terulete: %.2f cm² \n  Ossz terulet: %.2f cm² \n  Pixelek szama: %.0f pix\n", tt, ot / (pix * pix), ot);
	printf("  Kep merete: %d pixel x %d pixel\n", cinfo.image_width, cinfo.image_height );
//	printf("  Meret: %.0f x %.0f pix", sqrt(ot), sqrt(ot));
	
	/* wrap up decompression, destroy objects, free pointers and close open files */
	jpeg_finish_decompress( &cinfo );
	jpeg_destroy_decompress( &cinfo );
	free( row_pointer[0] );
	fclose( infile );
	/* yup, we succeeded! */
	return 1;
}
/**
 * write_jpeg_file Writes the raw image data stored in the raw_image buffer
 * to a jpeg image with default compression and smoothing options in the file
 * specified by *filename.
 *
 * \returns positive integer if successful, -1 otherwise
 * \param *filename char string specifying the file name to save to
 *
 */
 
int write_jpeg_file( char *filename ) {
	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr jerr;
	
	/* this is a pointer to one row of image data */
	JSAMPROW row_pointer[1];
	FILE *outfile = fopen( filename, "wb" );
	
	if ( !outfile )
	{
		printf(" Nem adtal meg kimeneti fajlt!\n A helyes argomentum sorrend: \n PixelSumConsole.exe <inputfajl> <erzekenyseg> <kapcsolo> <felbontas> <outputfajl> <kapcsolo2>\n Erzekenyseg 255 - 0 ig. (255 = feher szin, 0 = fekete szin)\n Kapcsolo: \"1\"  pix/cm\n           \"2\" pix/inch\n Felbontas vegeredmenyben pixel/cm\n Kapcsolo 2.: \"1\"  JPEG kep\n              \"2\"  RAW Kep (Csak egyenlo oldalu!)");
		return -1;
	}
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

int raw_analyse(char *ifilename, char *ofilename) {

	int i, s = 0;
	int c;

	FILE *infile = fopen( ifilename, "rb" );
	
	if ( !infile )
	{
		printf(" A bemeneti fajl nem letezik vagy nem nyithato meg!\n A helyes argomentum sorrend: \n PixelSumConsole.exe <inputfajl> <erzekenyseg> <kapcsolo> <felbontas> <outputfajl> <kapcsolo2>\n Erzekenyseg 255 - 0 ig. (255 = feher szin, 0 = fekete szin)\n Kapcsolo: \"1\"  pix/cm\n           \"2\" pix/inch\n Felbontas vegeredmenyben pixel/cm\n Kapcsolo 2.: \"1\"  JPEG kep\n              \"2\"  RAW Kep (Csak egyenlo oldalu!)");
		return -1;
	}

	FILE *outfile = fopen( ofilename, "wb" );
	
	if ( !outfile )
	{
		printf(" Nem adtal meg kimeneti fajlt!\n A helyes argomentum sorrend: \n PixelSumConsole.exe <inputfajl> <erzekenyseg> <kapcsolo> <felbontas> <outputfajl> <kapcsolo2>\n Erzekenyseg 255 - 0 ig. (255 = feher szin, 0 = fekete szin)\n Kapcsolo: \"1\"  pix/cm\n           \"2\" pix/inch\n Felbontas vegeredmenyben pixel/cm\n Kapcsolo 2.: \"1\"  JPEG kep\n              \"2\"  RAW Kep (Csak egyenlo oldalu!)");
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

	printf(" Kep adatai (.RAW):\n");

	printf("  Kornyezet: %.0f pix\n  Targy: %.0f pix \n", ot, tt);
	ot = ot + tt; // fehér + fekete pixelek száma = az összesével
	printf("  Felbontas: %.3f pix/cm\n", pix); // felbontás aránya
	printf("  Erzekenyseg: %d \n", sens);
	tt = tt / (pix * pix); // felbontás négyzetével (1 cm²) osztjuk a fekete pixelek számát  

	printf("  A targy terulete: %.2f cm² \n  Ossz terulet: %.2f cm² \n  Pixelek szama: %.0f pix\n", tt, ot / (pix * pix), ot);
	printf("  Meret: %.0f x %.0f pix", sqrt(ot), sqrt(ot));
	fclose(infile);
	fclose(outfile);
	
	return 1;
}

int main(int argc, char *argv[]) {
	char *infilename = argv[1], *outfilename = argv[5];
	
	if(argc < 2) {
		printf(" A programot kizarolag konzolbol lehet futtatni!\n Nem adtal meg a programnak argomentumot!\n A helyes argomentum sorrend: \n PixelSumConsole.exe <inputfajl> <erzekenyseg> <kapcsolo> <felbontas> <outputfajl> <kapcsolo2>\n Erzekenyseg 255 - 0 ig. (255 = feher szin, 0 = fekete szin)\n Kapcsolo: \"1\"  pix/cm\n           \"2\" pix/inch\n Felbontas vegeredmenyben pixel/cm\n Kapcsolo 2.: \"1\"  JPEG kep\n              \"2\"  RAW Kep (Csak egyenlo oldalu!)\n");
		system("pause");
        return 0; }
	
	if(getstr(argv[1], "help")) {
		printf(" A program kizarolag .JPG kiterjesztesu fajlokat kepes ertelmezni!\n A helyes argomentum sorrend: \n PixelSumConsole.exe <inputfajl> <erzekenyseg> <kapcsolo> <felbontas> <outputfajl> <kapcsolo2>\n Erzekenyseg 255 - 0 ig. (255 = feher szin, 0 = fekete szin)\n Kapcsolo: \"1\"  pix/cm\n           \"2\" pix/inch\n Felbontas vegeredmenyben pixel/cm\n Kapcsolo 2.: \"1\"  JPEG kep\n              \"2\"  RAW Kep (Csak egyenlo oldalu!)");
		return 0; }
	
	if(argc < 3) {
		printf(" Nem adtal meg erzekenysegi erteket!\n A helyes argomentum sorrend: \n PixelSumConsole.exe <inputfajl> <erzekenyseg> <kapcsolo> <felbontas> <outputfajl> <kapcsolo2>\n Erzekenyseg 255 - 0 ig. (255 = feher szin, 0 = fekete szin)\n Kapcsolo: \"1\"  pix/cm\n           \"2\" pix/inch\n Felbontas vegeredmenyben pixel/cm\n Kapcsolo 2.: \"1\"  JPEG kep\n              \"2\"  RAW Kep (Csak egyenlo oldalu!)");
		return 0; }
	
	sens = atoi(argv[2]);
	
	if(sens < 0 || sens > 255) {
		printf(" Helytelen erzekenysegi erteket adtal meg!\n A helyes argomentum sorrend: \n PixelSumConsole.exe <inputfajl> <erzekenyseg> <kapcsolo> <felbontas> <outputfajl> <kapcsolo2>\n Erzekenyseg 255 - 0 ig. (255 = feher szin, 0 = fekete szin)\n Kapcsolo: \"1\"  pix/cm\n           \"2\" pix/inch\n Felbontas vegeredmenyben pixel/cm\n Kapcsolo 2.: \"1\"  JPEG kep\n              \"2\"  RAW Kep (Csak egyenlo oldalu!)");
		return 0; }
		
	if(argc < 4) {
		printf(" Nem adtal meg kapcsolot!\n A helyes argomentum sorrend: \n PixelSumConsole.exe <inputfajl> <erzekenyseg> <kapcsolo> <felbontas> <outputfajl> <kapcsolo2>\n Erzekenyseg 255 - 0 ig. (255 = feher szin, 0 = fekete szin)\n Kapcsolo: \"1\"  pix/cm\n           \"2\" pix/inch\n Felbontas vegeredmenyben pixel/cm\n Kapcsolo 2.: \"1\"  JPEG kep\n              \"2\"  RAW Kep (Csak egyenlo oldalu!) (Csak egyenlo oldalu!)");
		return 0; }	

	kapcs = atoi(argv[3]);	
		
	if(kapcs < 1 || kapcs > 2) {
		printf(" Helytelen kapcsolot adtal meg!\n A helyes argomentum sorrend: \n PixelSumConsole.exe <inputfajl> <erzekenyseg> <kapcsolo> <felbontas> <outputfajl> <kapcsolo2>\n Erzekenyseg 255 - 0 ig. (255 = feher szin, 0 = fekete szin)\n Kapcsolo: \"1\"  pix/cm\n           \"2\" pix/inch\n Felbontas vegeredmenyben pixel/cm\n Kapcsolo 2.: \"1\"  JPEG kep\n              \"2\"  RAW Kep (Csak egyenlo oldalu!)");
		       return 0; }
			   
    if(argc < 5) {
		printf(" Nem adtel meg felbontasi erteket!\n A helyes argomentum sorrend: \n PixelSumConsole.exe <inputfajl> <erzekenyseg> <kapcsolo> <felbontas> <outputfajl> <kapcsolo2>\n Erzekenyseg 255 - 0 ig. (255 = feher szin, 0 = fekete szin)\n Kapcsolo: \"1\"  pix/cm\n           \"2\" pix/inch\n Felbontas vegeredmenyben pixel/cm\n Kapcsolo 2.: \"1\"  JPEG kep\n              \"2\"  RAW Kep (Csak egyenlo oldalu!)");
		return 0; }
		
	pix = atof(argv[4]);
	
	if(kapcs == 1) {
		if(pix < 0) { 
		       printf(" Helytelen felbontasi erteket adtal meg!\n A helyes argomentum sorrend: \n PixelSumConsole.exe <inputfajl> <erzekenyseg> <kapcsolo> <felbontas> <outputfajl> <kapcsolo2>\n Erzekenyseg 255 - 0 ig. (255 = feher szin, 0 = fekete szin)\n Kapcsolo: \"1\"  pix/cm\n           \"2\" pix/inch\n Felbontas vegeredmenyben pixel/cm\n Kapcsolo 2.: \"1\"  JPEG kep\n              \"2\"  RAW Kep (Csak egyenlo oldalu!)");
		       return 0; } }
	if(kapcs == 2) {
		pix = pix / INCH;
 	    if(pix < 0) { 
		       printf(" Helytelen felbontasi erteket adtal meg!\n A helyes argomentum sorrend: \n PixelSumConsole.exe <inputfajl> <erzekenyseg> <kapcsolo> <felbontas> <outputfajl> <kapcsolo2>\n Erzekenyseg 255 - 0 ig. (255 = feher szin, 0 = fekete szin)\n Kapcsolo: \"1\"  pix/cm\n           \"2\" pix/inch\n Felbontas vegeredmenyben pixel/cm\n Kapcsolo 2.: \"1\"  JPEG kep\n              \"2\"  RAW Kep (Csak egyenlo oldalu!)");
		       return 0; } }
			   
	if(argc < 6) {
		printf(" Nem adtal meg kimeneti fajlt!\n A helyes argomentum sorrend: \n PixelSumConsole.exe <inputfajl> <erzekenyseg> <kapcsolo> <felbontas> <outputfajl> <kapcsolo2>\n Erzekenyseg 255 - 0 ig. (255 = feher szin, 0 = fekete szin)\n Kapcsolo: \"1\"  pix/cm\n           \"2\" pix/inch\n Felbontas vegeredmenyben pixel/cm\n Kapcsolo 2.: \"1\"  JPEG kep\n              \"2\"  RAW Kep (Csak egyenlo oldalu!)");
		return 0; }
		
	if(argc < 7) {
		printf(" Nem adtal meg kapcsolot!\n A helyes argomentum sorrend: \n PixelSumConsole.exe <inputfajl> <erzekenyseg> <kapcsolo> <felbontas> <outputfajl> <kapcsolo2>\n Erzekenyseg 255 - 0 ig. (255 = feher szin, 0 = fekete szin)\n Kapcsolo: \"1\"  pix/cm\n           \"2\" pix/inch\n Felbontas vegeredmenyben pixel/cm\n Kapcsolo 2.: \"1\"  JPEG kep\n              \"2\"  RAW Kep (Csak egyenlo oldalu!)");
		return 0; }	

	kapcs2 = atoi(argv[6]);	
		
	if(kapcs2 < 1 || kapcs2 > 2) {
		printf(" Helytelen kapcsolot adtal meg!\n A helyes argomentum sorrend: \n PixelSumConsole.exe <inputfajl> <erzekenyseg> <kapcsolo> <felbontas> <outputfajl> <kapcsolo2>\n Erzekenyseg 255 - 0 ig. (255 = feher szin, 0 = fekete szin)\n Kapcsolo: \"1\"  pix/cm\n           \"2\" pix/inch\n Felbontas vegeredmenyben pixel/cm\n Kapcsolo 2.: \"1\"  JPEG kep\n              \"2\"  RAW Kep (Csak egyenlo oldalu!)");
		       return 0; }
			   
	if(kapcs2 == 1) {
		/* Try opening a jpeg*/
		if( read_jpeg_file( infilename ) > 0 ) 
		{
			/* then copy it to another file */
			if( write_jpeg_file( outfilename ) < 0 ) return -1;
		}
		else return -1;
		return 0; }
	if(kapcs == 2) { 
		if(raw_analyse(infilename, outfilename) < 0 ) {
			return 0; } }
}
