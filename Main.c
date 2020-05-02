/*

 *  Cartography main

 * LAP AMD-2020

 *

 * COMPILAÃ‡ÃƒO: gcc -std=c11 -o Main Cartography.c Main.c -lm

 */
#include "Cartography.h"
/*

 * Cartography module interface

 * LAP AMD-2020

 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
/* STRING -------------------------------------- */
typedef char String[256];
typedef String StringVector[1024];
/* IDENTIFICATION -------------------------------------- */
typedef struct { // IdentificaÃ§Ã£o duma parcela
 String freguesia, concelho, distrito;
} Identification;
/* COORDINATES -------------------------------------- */
typedef struct { // Coordenadas - latitude e longitude
 double lat, lon;
} Coordinates;
Coordinates coord(double lat, double lon);
bool sameCoordinates(Coordinates c1, Coordinates c2);
double haversine(Coordinates c1, Coordinates c2);
/* RECTANGLE -------------------------------------- */
typedef struct { // Retangulo
 Coordinates topLeft, bottomRight;
} Rectangle;
Rectangle rect(Coordinates tl, Coordinates br);
bool insideRectangle(Coordinates c, Rectangle r);
bool rectangleIntersect(Rectangle a, Rectangle b);
/* RING -------------------------------------- */
typedef struct { // Anel - um caminho linear fechado que nÃ£o se auto-intersecta
 Coordinates vertexes[30000];
 int nVertexes;
 Rectangle boundingBox;
} Ring;
bool insideRing(Coordinates c, Ring r);
bool adjacentRings(Ring a, Ring b);
/* PARCEL -------------------------------------- */
typedef struct { // Parcela duma freguesia. Ã‰ um anel com eventuais buracos
 Identification identification;
 Ring edge;
 Ring holes[2];
 int nHoles;
} Parcel;
bool insideParcel(Coordinates c, Parcel p);
bool adjacentParcels(Parcel a, Parcel b);
/* CARTOGRAPHY -------------------------------------- */
typedef Parcel Cartography[200]; // Mapa - uma coleÃ§Ã£o de parcelas
int loadCartography(String fileName, Cartography cartography);
void showCartography(Cartography cartography, int n);
/* INTERPRETER -------------------------------------- */
void interpreter(Cartography cartography, int n);
#include <assert.h>
#include <malloc.h>
static void internalTests(bool show)
{
	char * cc = "Ola David";
	if( show ) {
		assert( fabs( haversine(coord(36.12, -86.67), coord(33.94, -118.40))
				- 2886.444442837983 ) < 0.00000000001 );
	
		Ring r = {{{0,0}, {0,1}, {1, 1}, {1,0}}, 4, {{1,0}, {0,1}}};
		assert( insideRing(coord(0.5, 0.5), r) );
		assert( insideRing(coord(0.0000001, 0.0000001), r) );
		assert( insideRing(coord(0.0000001, 0.9999999), r) );
		assert( insideRing(coord(0.9999999, 0.9999999), r) );
		assert( !insideRing(coord(1.0000001, 0.5), r) );
		assert( !insideRing(coord(0.5, 1.0000001), r) );
	}
}
static Cartography cartography; // variÃ¡vel gigante
static int nCartography = 0;
static void showMemory(bool show)
{
 if( show ) {
  long staticMemory = sizeof(cartography);
  printf("static memory = %ldMB\n", staticMemory/1024/1024);
 }
}
int main(void)
{
 internalTests(false);
 nCartography = loadCartography("map.txt", cartography);
 showMemory(false);
 showCartography(cartography, nCartography);
 interpreter(cartography, nCartography);
 return 0;
}
