/*
largura maxima = 100 colunas
tab = 4 espaÃ§os
0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789

	Linguagens e Ambientes de ProgramaÃ§Ã£o (B) - Projeto de 2019/20

	Cartography.c

	Este ficheiro constitui apenas um ponto de partida para o
	seu trabalho. Todo este ficheiro pode e deve ser alterado
	Ã  vontade, a comeÃ§ar por este comentÃ¡rio. Ã‰ preciso inventar
	muitas funÃ§Ãµes novas.

COMPILAÃ‡ÃƒO

  gcc -std=c11 -o Main Cartography.c Main.c -lm

IDENTIFICAÃ‡ÃƒO DOS AUTORES

  Aluno 1: 55045 David Antunes
  Aluno 2: 55797 Joao Daniel

COMENTÃRIO

 Coloque aqui a identificaÃ§Ã£o do grupo, mais os seus comentÃ¡rios, como
 se pede no enunciado.

*/
#define USE_PTS		true
#include "Cartography.h"
/* STRING -------------------------------------- */

static void showStringVector(StringVector sv, int n)
{
	int i;
	for (i = 0; i < n; i++)
	{
		printf("%s\n", sv[i]);
	}
}
typedef bool BoolFun(Coordinates, Coordinates);

/* UTIL */

static void error(String message)
{
	fprintf(stderr, "%s.\n", message);
	exit(1); // Termina imediatamente a execuÃ§Ã£o do programa
}

static void readLine(String line, FILE *f) // lÃª uma linha que existe obrigatoriamente
{
	if (fgets(line, MAX_STRING, f) == NULL)
		error("Ficheiro invalido");
	line[strlen(line) - 1] = '\0'; // elimina o '\n'
}

static int readInt(FILE *f)
{
	int i;
	String line;
	readLine(line, f);
	sscanf(line, "%d", &i);
	return i;
}

/* IDENTIFICATION -------------------------------------- */

static Identification readIdentification(FILE *f)
{
	Identification id;
	String line;
	readLine(line, f);
	sscanf(line, "%s %s %s", id.freguesia, id.concelho, id.distrito);
	return id;
}

static void showIdentification(int pos, Identification id, int z)
{
	if (pos >= 0) // pas zero interpretado como nÃ£o mostrar
		printf("%4d ", pos);
	else
		printf("%4s ", "");
	if (z == 3)
		printf("%-25s %-13s %-22s", id.freguesia, id.concelho, id.distrito);
	else if (z == 2)
		printf("%-25s %-13s %-22s", "", id.concelho, id.distrito);
	else
		printf("%-25s %-13s %-22s", "", "", id.distrito);
}

static void showValue(int value)
{
	if (value < 0) // value negativo interpretado como char
		printf(" [%c]\n", -value);
	else
		printf(" [%3d]\n", value);
}

static bool sameIdentification(Identification id1, Identification id2, int z)
{
	if (z == 3)
		return strcmp(id1.freguesia, id2.freguesia) == 0 
		&& strcmp(id1.concelho, id2.concelho) == 0 
		&& strcmp(id1.distrito, id2.distrito) == 0;
	else if (z == 2)
		return strcmp(id1.concelho, id2.concelho) == 0 
		&& strcmp(id1.distrito, id2.distrito) == 0;
	else
		return strcmp(id1.distrito, id2.distrito) == 0;
}

/* COORDINATES -------------------------------------- */

Coordinates coord(double lat, double lon)
{
	Coordinates c = {lat, lon};
	return c;
}

static Coordinates readCoordinates(FILE *f)
{
	double lat, lon;
	String line;
	readLine(line, f);
	sscanf(line, "%lf %lf", &lat, &lon);
	return coord(lat, lon);
}

bool sameCoordinates(Coordinates c1, Coordinates c2)
{
	return c1.lat == c2.lat && c1.lon == c2.lon;
}

static double toRadians(double deg)
{
	return deg * PI / 180.0;
}

// https://en.wikipedia.org/wiki/Haversine_formula
double haversine(Coordinates c1, Coordinates c2)
{
	double dLat = toRadians(c2.lat - c1.lat);
	double dLon = toRadians(c2.lon - c1.lon);

	double sa = sin(dLat / 2.0);
	double so = sin(dLon / 2.0);

	double a = sa * sa + so * so * cos(toRadians(c1.lat)) * cos(toRadians(c2.lat));
	return EARTH_RADIUS * (2 * asin(sqrt(a)));
}

/* RECTANGLE -------------------------------------- */

Rectangle rect(Coordinates tl, Coordinates br)
{
	Rectangle r = {tl, br};
	return r;
}

static void showRectangle(Rectangle r)
{
	printf("{%lf, %lf, %lf, %lf}",
		   r.topLeft.lat, r.topLeft.lon,
		   r.bottomRight.lat, r.bottomRight.lon);
}

static Rectangle calculateBoundingBox(Coordinates vs[], int n)
{
	double tx = vs[0].lat;
	double ty = vs[0].lon;
	double bx = vs[0].lat;
	double by = vs[0].lon;

	for (int i = 1; i < n; i++)
	{
		if (vs[i].lat > tx)
			tx = vs[i].lat;
		if (vs[i].lon < ty)
			ty = vs[i].lon;
		if (vs[i].lat < bx)
			bx = vs[i].lat;
		if (vs[i].lon > by)
			by = vs[i].lon;
	}

	return rect(coord(tx, ty), coord(bx, by));
}

bool insideRectangle(Coordinates c, Rectangle r)
{
	if(c.lat <= r.topLeft.lat 
	&& r.topLeft.lon <= c.lon 
	&& r.bottomRight.lat <= c.lat 
	&& c.lon <= r.bottomRight.lon)
	return true;

	return false;
}

/* RING -------------------------------------- */

static Ring readRing(FILE *f)
{
	Ring r;
	int i, n = readInt(f);
	r.nVertexes = n;
	r.vertexes = (Coordinates *) malloc(sizeof(Coordinates)*n);
	for (i = 0; i < n; i++)
	{
		 r.vertexes[i] = readCoordinates(f);
	}
	r.boundingBox =
		calculateBoundingBox(r.vertexes, r.nVertexes);
	return r;
}

// http://alienryderflex.com/polygon/
bool insideRing(Coordinates c, Ring r)
{
	if (!insideRectangle(c, r.boundingBox)) // otimizaÃ§Ã£o
		return false;
	double x = c.lon, y = c.lat;
	int i, j;
	bool oddNodes = false;
	for (i = 0, j = r.nVertexes - 1; i < r.nVertexes; j = i++)
	{
		double xi = r.vertexes[i].lon, yi = r.vertexes[i].lat;
		double xj = r.vertexes[j].lon, yj = r.vertexes[j].lat;
		if (((yi < y && y <= yj) || (yj < y && y <= yi)) && (xi <= x || xj <= x))
		{
			oddNodes ^= (xi + (y - yi) / (yj - yi) * (xj - xi)) < x;
		}
	}
	return oddNodes;
}

/**
 * checks if the ring has a coordinate exactly the same as c1
 * */
static bool vertexInRing(Coordinates c1, Ring r)
{
	int nOuters = r.nVertexes;
	Coordinates *outers = r.vertexes;
	int i = 0;
	while (i < nOuters)
	{
		if (outers[i].lat == c1.lat && outers[i].lon == c1.lon)
		{
			return true;
		}
		i++;
	}
	return false;
}
/* 
given two rings a and b, checks if the have at least one common vertex
**/
bool adjacentRings(Ring a, Ring b)
{
	int n = a.nVertexes;
	Coordinates *verts = a.vertexes;
	for (int i = 0; i < n; i++)
	{
		if (vertexInRing(verts[i], b))
		{
			return true;
		}
	}
	return false;
}

/* PARCEL -------------------------------------- */

static Parcel readParcel(FILE *f)
{
	Parcel p;
	p.identification = readIdentification(f);
	int i, n = readInt(f);
	p.edge = readRing(f);
	p.nHoles = n;
	p.holes = (Ring *) malloc(sizeof(Ring)*n);
	for (i = 0; i < n; i++)
	{
		p.holes[i] = readRing(f);
	}
	return p;
}

static void showHeader(Identification id)
{
	showIdentification(-1, id, 3);
	printf("\n");
}

static void showParcel(int pos, Parcel p, int lenght)
{
	showIdentification(pos, p.identification, 3);
	showValue(lenght);
}
/**checks if the coordinates c is among the coordinates of the parcel p
 * */
bool insideParcel(Coordinates c, Parcel p)
{
	if(insideRing(c, p.edge))
	{
		if(p.nHoles != 0) {
		for(int i = 0; i < p.nHoles; i++)
			if(insideRing(c, p.holes[i]))
				return false;
		}
		return true;
	}
	return false;
}

/* given two parcels, checks if they have at least a common coordinate */
bool adjacentParcels(Parcel a, Parcel b)
{
	//checks if a's outer edges and b outer edges the same share a coordinate
	if (adjacentRings(a.edge, b.edge)) 
	{
		return true;
	}

	int nHoles = a.nHoles;

	int bnHoles = b.nHoles;

	//checks if a's outer edges and b's holes share a coordinate
	for (int i = 0; i < bnHoles; i++) 
	{
		if(adjacentRings(a.edge,b.holes[i])){
			return true;
		}
	}
	//checks if b's outer edges and a's holes share a coordinate
	for (int i = 0; i < nHoles; i++)
	{
		if(adjacentRings(a.holes[i],b.edge)){
			return true;
		}
	}
	return false;
}


/* CARTOGRAPHY -------------------------------------- */
int loadCartography(String fileName, Cartography *cartography)
{
	FILE *f;
	int i;
	f = fopen(fileName, "r");
	if (f == NULL)
		error("Impossivel abrir ficheiro");
	int n = readInt(f);
	*cartography = malloc(sizeof(Parcel)*n);
	for (i = 0; i < n; i++)
	{
		(*cartography)[i] = readParcel(f);
	}
	fclose(f);
	return n;
}

static int findLast(Cartography cartography, int n, int j, Identification id)
{
	for (; j < n; j++)
	{
		if (!sameIdentification(cartography[j].identification, id, 3))
			return j - 1;
	}
	return n - 1;
}

void showCartography(Cartography cartography, int n)
{
	int last;
	Identification header = {"__FREGUESIA__", "__CONCELHO__", "__DISTRITO__"};
	showHeader(header);
	for (int i = 0; i < n; i = last + 1)
	{
		last = findLast(cartography, n, i, cartography[i].identification);
		showParcel(i, cartography[i], last - i + 1);
	}
}

/* INTERPRETER -------------------------------------- */

static bool checkArgs(int arg)
{
	if (arg != -1)
		return true;
	else
	{
		printf("ERRO: FALTAM ARGUMENTOS!\n");
		return false;
	}
}

static bool checkPos(int pos, int n)
{
	if (0 <= pos && pos < n)
		return true;
	else
	{
		printf("ERRO: POSICAO INEXISTENTE!\n");
		return false;
	}
}

/* QSORT COMPARATORS -------------------------------------- */

static int v_strcmp(const void *str1, const void *str2)
{
	return strcmp(str1, str2);
}

static int v_cmp(const void *str1, const void *str2)
{
	return (*(int*)str1) - (*(int*)str2);
}

/* SINGLE LINKED LIST STRUCTURE -------------------------------------- */
typedef struct Node
{
	int data;
	struct Node *next;
} Node, *List;

static List newNode(int val, List next)
{
	List n = malloc(sizeof(Node));
	if (n == NULL)
		return NULL;
	n->data = val;
	n->next = next;
	return n;
}
List listPutAtEnd(List l, int val)
{
	if (l == NULL)
		return newNode(val, NULL);
	else
	{
		List p;
		for (p = l; p->next != NULL; p = p->next)
			;						  // Stop at the last node
		p->next = newNode(val, NULL); // Assign to the next of the last node
		return l;
	}
}

// L
static void commandListCartography(Cartography cartography, int n)
{
	showCartography(cartography, n);
}

/* 
	Given a parcel, it counts the total number of vertexes present in the edge and all of the holes
*/
static int getTotalVertexes(Parcel p)
{
	int counter = p.edge.nVertexes;

	for (int i = 0; i < p.nHoles; i++)
	{
		counter += p.holes[i].nVertexes;
	}
	return counter;
}

	/* 
		Depending on the value of reverse, it will search for the maximum number of vertexes 
		from the given pos, altering the value in memory of position and max.
		IF reverse is true, the function will go backwards from pos.
		IF reverse is false, the function will go forward from pos.	

	 */
static void searchMax(Cartography cartography, int n, int *max, int pos, int * position, bool reverse)
{
	int i =  pos;
	Parcel maxParcel = cartography[pos];

		while(sameIdentification(maxParcel.identification, cartography[i].identification, 3))
	{

		int maxVertexes = getTotalVertexes(cartography[i]);
		if (*max < maxVertexes) // If maxVertexes is bigger, max is changed with the new value
		{
			*max = maxVertexes;
			maxParcel = cartography[i];
			*position = i;
		}
		
		if(reverse)
			i--;
		else
		{
			i++;
		}
		if(i == -1 || i == n)
			break;
		
	}
}
/*
	Given the position, it will print the maximum number of vertexes present in the parcel with the same name
*/
static void commandMaximum(int pos, Cartography cartography, int n)
{
	if (!checkArgs(pos) || !checkPos(pos, n))
	{
		return;
	}
	int max = getTotalVertexes(cartography[pos]);
	int position = pos;

	// Parses the cartography in a decreasing index
	searchMax(cartography, n, &max, pos, &position, true);
	// Parses the cartography in a increasing index
	searchMax(cartography, n, &max, pos, &position, false);	

	showParcel(position, cartography[position], max);

}


/*boolean type struct */
typedef bool BoolFun(Coordinates, Coordinates);

/* given two different coordinates returns true if c1 is north relative to c2 */
static bool northest(Coordinates c1, Coordinates c2)
{
	return c1.lat >= c2.lat;
}
/* given two different coordinates returns true if c1 is south relative to c2 */
static bool southest(Coordinates c1, Coordinates c2)
{
	return !northest(c1, c2);
}
/* given two different coordinates returns true if c1 is east relative to c2 */
static bool easthern(Coordinates c1, Coordinates c2)
{
	return c1.lon >= c2.lon;
}

/* given two different coordinates returns true if c1 is west relative to c2 */
static bool westest(Coordinates c1, Coordinates c2)
{
	return !easthern(c1, c2);
}
/** given a parcel, and 4 bool function that says who is north, south, west and north,
 *  it saves in the array res the coordinates of 
 * the northest, southest, eastern and western edges in array given as an argument  */
static void extremeCoordinates(Parcel p1, BoolFun north, BoolFun south, 
								BoolFun east, BoolFun west, Coordinates res[])
{
	Ring r = p1.edge;
	Coordinates no = r.vertexes[0];
	Coordinates s = r.vertexes[0];
	Coordinates e = r.vertexes[0];
	Coordinates w = r.vertexes[0];

	int len = r.nVertexes;

	for (int i = 1; i < len; i++)
	{
		if (north(r.vertexes[i], no))
		{
			no = r.vertexes[i];
		}
		if (south(r.vertexes[i], s))
		{
			s = r.vertexes[i];
		}
		if (east(r.vertexes[i], e))
		{
			e = r.vertexes[i];
		}
		if (west(r.vertexes[i], w))
		{
			no = r.vertexes[i];
		}
	}
	res[0] = no;
	res[1] = s;
	res[2] = e;
	res[3] = w;
}

// bol: north south east west

// bol: north south east west
/* prints the parcel from the cartography carts that has the nearest coordinates to the cardinal points */
void extremeParcel(Cartography carts, int len, BoolFun north, 
					BoolFun south, BoolFun east, BoolFun west)
{
	Parcel p1 = carts[0];
	Parcel p2 = carts[0];
	Parcel p3 = carts[0];
	Parcel p4 = carts[0];

	int pos1 = 0;
	int pos2 = 0;
	int pos3 = 0;
	int pos4 = 0;


	Coordinates cs[4];
	extremeCoordinates(p1, north, south, east, west, cs);
	Coordinates n = cs[0];
	Coordinates s = cs[1];
	Coordinates e = cs[2];
	Coordinates w = cs[3];

	Coordinates aux;
	for (int i = 1; i < len; i++)
	{
		extremeCoordinates(carts[i], north, south, east, west, cs);
		aux = cs[0];
		if (north(aux, n))
		{ // if c1 the northest
			p1 = carts[i];
			n = aux;
			pos1 = i;
		}
		aux = cs[1];
		if (south(aux, s))
		{ //compare the southest
			p2 = carts[i];
			s = aux;
			pos2 = i;
		}
		aux = cs[2];
		if (east(aux, e))
		{ //compare the eastern
			p3 = carts[i];
			e = aux;
			pos3 = i;
		}
		aux = cs[3];
		if (west(aux, w))
		{ //compare the westest
			p4 = carts[i];
			w = aux;
			pos4 = i;
		}
	}

	showParcel(pos1, p1, -'N');
	showParcel(pos3, p3, -'E');
	showParcel(pos2, p2, -'S');
	showParcel(pos4, p4, -'W');
}

static void commandParcelExtremes(Cartography cartography, int n)
{

	if (n == 0 || cartography == NULL)
	{
		printf("ERRO: MAPA VAZIO!\n");
		return;
	}

	extremeParcel(cartography, n, northest, southest, easthern, westest);
}

/*
	Prints the information of a given position
	The information consists of the number of vertexes in the edge
	and if there are any holes, it will also print the vertexes in the holes.
	Finally it will print the boundingbox that delimits the parcel
*/
static void commandResume(int pos, Cartography cartography, int n)
{
	Parcel p = cartography[pos];

	showIdentification(pos, p.identification, 3);

	printf("\n%4s %d ", "", p.edge.nVertexes);

	if (p.nHoles != 0)
	{
		for (int i = 0; i < p.nHoles; i++)
		{
			printf("%d ", p.holes[i].nVertexes);
		}
	}
	showRectangle(p.edge.boundingBox);
	printf("\n");
}


/*
	Given a pair of coordinates and a position of a parcel, 
	it will print the distance between the given coordinates and the nearest 
	vertexes present in the edge of the parcel
*/
static void commandTravel(double lat, double lon, int pos, Cartography cartography, int n)
{
	if (!checkPos(pos, n))
	{
		return;
	}

	Coordinates c1 = {lat, lon};

	Parcel p = cartography[pos];

	int len = p.edge.nVertexes;
	Coordinates *cs = p.edge.vertexes;
	double res = haversine(c1, cs[0]);
	double aux;


	//Searches for the nearest vertex 
	for (int i = 1; i < len; i++)
	{
		aux = haversine(c1, cs[i]);
		if (aux < res)
		{
			res = aux;
		}
	}
	printf(" %f\n", res);
}


/*
	Prints the number of towns with the same name,
	then prints the number of counties with the same name,
	then finally, prints the number of districts with the same name.
*/
static void commandHowMany(int pos, Cartography cartography, int n)


{
	if (!checkPos(pos, n))
	{
		return;
	}

	int towns = 0;
	int counties = 0;
	int districts = 0;

	for(int i = 0; i < n; i++)
	{
		if (sameIdentification(cartography[pos].identification, cartography[i].identification, 3))
		{
			towns++;
			counties++;
			districts++;
		}
		else if(sameIdentification(cartography[pos].identification, cartography[i].identification, 2))
		{
			counties++;
			districts++;
		}
		else if(sameIdentification(cartography[pos].identification, cartography[i].identification, 1))
		{
			districts++;
		}
	}

	//Freguesias
	showParcel(pos, cartography[pos], towns);
	//Concelhos
	showIdentification(pos, cartography[pos].identification, 2);
	showValue(counties);
	//Distritos
	showIdentification(pos, cartography[pos].identification, 1);
	showValue(districts);
}


/*
	Searches inside of the string vector if there is an index with the given name
*/
static bool hasName(String name, StringVector sv, int n)
{
	for(int i = 0; i < n; i++)
	{
		//If it is the same strcmp returns 0 thus needing !
		if (!strcmp(name, sv[i]))
			return true;
	}
	return false;
}
/*
	Prints the names of the counties present in the cartography in alphabetical order
*/
static void commandCounties(Cartography cartography, int n)
{
	StringVector counties;
	strcpy(counties[0], cartography[0].identification.concelho);

	int i = 0;
	int ncounties = 0;

	while (i < n)
	{
		if (!hasName(cartography[i].identification.concelho, counties, ncounties))
		{
			strcpy(counties[ncounties++], cartography[i].identification.concelho);
		}
		i++;
	}

	qsort(counties, ncounties, sizeof(String), v_strcmp);
	showStringVector(counties, ncounties);
}

/*
	Prints the names of the districts present in the cartography in alphabetical order
*/
static void commandDistricts(Cartography cartography, int n)
{
	StringVector districts;
	strcpy(districts[0], cartography[0].identification.distrito);

	int i = 0;
	int ndistricts = 0;

	while (i < n)
	{
		if (!hasName(cartography[i].identification.distrito, districts, ndistricts))
		{
			strcpy(districts[ndistricts++], cartography[i].identification.distrito);
		}
		i++;
	}

	qsort(districts, ndistricts, sizeof(String), v_strcmp);
	showStringVector(districts, ndistricts);
}
/*
	Prints which parcel has inside the given coordinates
*/
static void commandParcel(double lat, double lon, Cartography cartography, int n)
{
	bool found = false;
	int pos = -1;

	for (int i = 0; i < n && !found; i++)
	{
		Coordinates cords = coord(lat, lon);
		if (insideParcel(cords, cartography[i]))
		{
			found = true;
			pos = i;
		}
	}

	if (found)
	{
		showIdentification(pos, cartography[pos].identification, 3);
		printf("\n");
	}
	else
	{
		printf("FORA DO MAPA\n");
	}
}

/*
	Prints the parcel that are adjacent to the given parcel with the given pos
*/

static void CommandAdjecent(int pos, Cartography cartography, int n)
{
	if (!checkPos(pos, n))
	{
		return;
	}
	int position[n]; //Position of the adjacent parcel
	int counter = 0; // Number of parcels adjacent

	int j = 0;

	while (j < n)
	{
		if (j != pos && adjacentParcels(cartography[pos], cartography[j]))
		{
			position[counter++] = j;
		}
		j++;
	}
	if (counter == 0)
	{
		printf("NAO HA ADJACENCIAS\n");
		return;
	}
	
	for (int i = 0; i < counter; i++)
	{
		showIdentification(position[i], cartography[position[i]].identification, 3);
		printf("\n");
	}
}

/* performs a breadth search to find the number of paths from the parcel src to dest*/
static int bfs(Cartography carts, int n, int src, int dest)
{
	int dist[n]; //stores distance of every parcel from src
	int queue[n]; // keeps track of all the visited parcels
	memset(dist, -1, sizeof(int) * n);
	memset(queue, -1, sizeof(int) * n);

	dist[src] = 0; 
	queue[0] = src; 

	int elems = 1; //number os elements added to the queue
	int pop = 0; //number of elements removed from the queue
	int current = src; //current as the father node during the iteration
	
	while (elems < n && pop < elems ) // while there are more elements to be queued and not all queue were removed
	{
		current = queue[pop++]; 
		/*
		David, We dont need this
		if (current >= n || current < 0)
		{
			break;
		} */

		for (int i = 0; i < n; i++) //finds all the neighbours of current
		{
			if (current != i && adjacentParcels(carts[current], carts[i])) 
			{
				if (dist[i]== -1) // if it was not visited yet
				{
					if (i != dest) //if it is not the parcel we want
					{
						queue[elems++] = i; //add it into the queue to have its neighbours checked
						dist[i] = dist[current] + 1; //calculates its distance from the src
					}
					else
					{	//if we found the target, return its distance from the source
						return dist[current] + 1;
					}
				}
			}
		}
	}

	return 0;
}

/*
	Prints the number of frontiers the user has to pass to go from pos1 to pos2
*/
static void commandFrontier(int pos1, int pos2, Cartography cartography, int n)
{
	if (!checkPos(pos1, n) || !checkPos(pos2, n))
	{
		return;
	}
	int res = 0;

	if (pos1 == pos2)
	{
		res = 0;
		printf(" %d\n", res);
	}
	else if (adjacentParcels(cartography[pos1], cartography[pos2]))
	{
		res = 1;
		printf(" %d\n", res);
	} 
	else
	{
		res = bfs(cartography, n, pos1, pos2);
		if (res == 0)
		{
			printf("NAO HA CAMINHO\n");
		}
		else
		{
			printf(" %d\n", res);
		}
	}
}

static void printSequence(int start, int end)
{
	if(start != end)
		printf(" %d-%d", start, end);
	else
	printf(" %d", start);
}

static int getPartitions(double distance, List result[], Cartography cartography, int n)
{
	int resCounter = 0; //Number of subsets
	List subA = NULL; 
	int added[n]; // Number of parcels added in the subset

	memset(added, 0, n * sizeof(int));

	subA = NULL;
	for (int k = 0; k < n; k++)
	{	
		//Checks if the parcel is already used
		if (added[k] == 0)
		{
			//Creates a new subset
			added[k] = 1;
			subA = listPutAtEnd(subA, k);

			//Checks for every parcel in the list if there are more parcels that can be add
			for(List aux = subA; aux != NULL; aux = aux->next)
			{
				for (int z = 0; z < n; z++)
				{
					if (added[z] == 0 && z != k)
					{
						if (distance >= haversine(cartography[aux->data].edge.vertexes[0], 
												cartography[z].edge.vertexes[0]))
						{
								listPutAtEnd(aux, z);
								added[z] = 1;
						}
					}
				}
			}

			// Stores the subset 
			if (subA != NULL)
			{
				result[resCounter] = subA;
				resCounter++;
				subA = NULL;
			}
		}
		subA = NULL;
	}
	return resCounter;
}

static int listToVector(List list, int * values)
{
	List subA = list;
	int counter = 0;
	//Converts the list to vector

		for(; subA != NULL; subA = subA->next)
		{
			values[counter++] = subA->data;
			free(subA);
		}
	return counter;
}

/*
	Partitions the parcels using the given distance, 
	thus any parcel of a partition cant reach 
	another parcel in an another partition
*/
static void commandPartition(double distance, Cartography cartography, int n)
{

	List result [n];//Lists of every subset
	int resCounter = getPartitions(distance, result, cartography, n); //Number of subsets

	int values[n];
	int counter = 0;

	for (int k = 0; k < resCounter; k++)
	{

		counter = listToVector(result[k],values);

		//Sorts the vector
		qsort(values, counter, sizeof(int), v_cmp);	

		int start = values[0];
		int end = values[0];

		//Prints each subset
		for (int i = 0; i < counter; i++)
		{
			//Checks if the next position is invalid
			if((i + 1) < counter)
			{
				//Checks if the next one is the succession of end 
				if(values[i + 1] - end == 1) 
				{
					end = values[i + 1];
				}

				else
				{	
					printSequence(start,end);
					//Changes to the next value in the vector
					start = values[i+ 1];
					end = values[i+ 1];
				}

			}
			else
			{
				printSequence(start,end);
			}
		}
		counter = 0;
		printf("\n");
	}
}

static void freeAll(Cartography cartography, int n)
{
	for(int i = 0; i < n; i++)
	{
		for(int j = 0; j < cartography[i].nHoles; j++)
			free(cartography[i].holes[j].vertexes);
	free(cartography[i].holes);
	free(cartography[i].edge.vertexes);
	}
	free(cartography);
}

void interpreter(Cartography cartography, int n)
{
	String commandLine;
	for (;;)
	{ // ciclo infinito
		printf("> ");
		readLine(commandLine, stdin);
		char command = ' ';
		double arg1 = -1.0, arg2 = -1.0, arg3 = -1.0;
		sscanf(commandLine, "%c %lf %lf %lf", &command, &arg1, &arg2, &arg3);
		// printf("%c %lf %lf %lf\n", command, arg1, arg2, arg3);
		switch (commandLine[0])
		{
		case 'L':
		case 'l': // listar
			commandListCartography(cartography, n);
			break;

		case 'M':
		case 'm': // maximo
			commandMaximum(arg1, cartography, n);
			break;

		case 'X':
		case 'x': // extremos
			commandParcelExtremes(cartography, n);
			break;

		case 'R':
		case 'r': // resumo
			commandResume(arg1, cartography, n);
			break;

		case 'V':
		case 'v': // viagem
			commandTravel(arg1, arg2, arg3, cartography, n);
			break;

		case 'Q':
		case 'q': // quantos
			commandHowMany(arg1, cartography, n);
			break;

		case 'C':
		case 'c': // concelhos
			commandCounties(cartography, n);
			break;

		case 'D':
		case 'd': // distritos
			commandDistricts(cartography, n);
			break;

		case 'P':
		case 'p': // parcela
			commandParcel(arg1, arg2, cartography, n);
			break;

		case 'A':
		case 'a': // adjacencia
			CommandAdjecent(arg1, cartography, n);
			break;

		case 'F':
		case 'f': // fronteira
			commandFrontier(arg1, arg2, cartography, n);
			break;

		case 'T':
		case 't': // particao
			commandPartition(arg1, cartography, n);
			break;

		case 'Z':
		case 'z': // terminar
			printf("Fim de execucao! Volte sempre.\n");
			freeAll(cartography, n);
			return;

		default:
			printf("Comando desconhecido: \"%s\"\n", commandLine);
		}
	}
}