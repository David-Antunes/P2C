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

  Aluno 1: nÃºmero1 nome1
  Aluno 2: nÃºmero2 nome2

COMENTÃRIO

 Coloque aqui a identificaÃ§Ã£o do grupo, mais os seus comentÃ¡rios, como
 se pede no enunciado.

*/

#include "Cartography.h"
/* STRING -------------------------------------- */

static void showStringVector(StringVector sv, int n) {
	int i;
	for( i = 0 ; i < n ; i++ ) {
		printf("%s\n", sv[i]);
	}
}
typedef bool BoolFun(Coordinates,Coordinates);

/* UTIL */

static void error(String message)
{
	fprintf(stderr, "%s.\n", message);
	exit(1);	// Termina imediatamente a execuÃ§Ã£o do programa
}

static void readLine(String line, FILE *f)	// lÃª uma linha que existe obrigatoriamente
{
	if( fgets(line, MAX_STRING, f) == NULL )
		error("Ficheiro invalido");
	line[strlen(line) - 1] = '\0';	// elimina o '\n'
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
	if( pos >= 0 ) // pas zero interpretado como nÃ£o mostrar
		printf("%4d ", pos);
	else
		printf("%4s ", "");
	if( z == 3 )
		printf("%-25s %-13s %-22s", id.freguesia, id.concelho, id.distrito);
	else if( z == 2 )
		printf("%-25s %-13s %-22s", "", id.concelho, id.distrito);
	else
		printf("%-25s %-13s %-22s", "", "", id.distrito);
}

static void showValue(int value)
{
	if( value < 0 ) // value negativo interpretado como char
		printf(" [%c]\n", -value);
	else
		printf(" [%3d]\n", value);
}

static bool sameIdentification(Identification id1, Identification id2, int z)
{
	if( z == 3 )
		return strcmp(id1.freguesia, id2.freguesia) == 0
			&& strcmp(id1.concelho, id2.concelho) == 0
			&& strcmp(id1.distrito, id2.distrito) == 0;
	else if( z == 2 )
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
	Rectangle rect = {tl, br};
	return rect;
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

	for(int i = 1; i < n; i++)
	{
		if(vs[i].lat > tx)
			tx = vs[i].lat;
		if(vs[i].lon < ty)
			ty = vs[i].lon;
		if(vs[i].lat < bx)
			bx = vs[i].lat;
		if(vs[i].lon > by)
			by = vs[i].lon;	
	}

	return rect(coord(tx,ty), coord(bx,by));
}

bool insideRectangle(Coordinates c, Rectangle r)
{
	////// FAZER
		return false;
}


/* RING -------------------------------------- */

static Ring readRing(FILE *f)
{
	Ring r;
	int i, n = readInt(f);
	if( n > MAX_VERTEXES )
		error("Anel demasiado extenso");
	r.nVertexes = n;
	for( i = 0 ; i < n ; i++ ) {
		r.vertexes[i] = readCoordinates(f);
	}
	r.boundingBox =
		calculateBoundingBox(r.vertexes, r.nVertexes);
	return r;
}


// http://alienryderflex.com/polygon/
bool insideRing(Coordinates c, Ring r)
{
	if( !insideRectangle(c, r.boundingBox) )	// otimizaÃ§Ã£o
		return false;
	double x = c.lon, y = c.lat;
	int i, j;
	bool oddNodes = false;
	for( i = 0, j = r.nVertexes - 1 ; i < r.nVertexes ; j = i++ ) {
		double xi = r.vertexes[i].lon, yi = r.vertexes[i].lat;
		double xj = r.vertexes[j].lon, yj = r.vertexes[j].lat;
		if( ((yi < y && y <= yj) || (yj < y && y <= yi))
								&& (xi <= x || xj <= x) ) {
			oddNodes ^= (xi + (y-yi)/(yj-yi) * (xj-xi)) < x;
		}
	}
	return oddNodes;
}




/* PARCEL -------------------------------------- */

static Parcel readParcel(FILE *f)
{
	Parcel p;
	p.identification = readIdentification(f);
	int i, n = readInt(f);
	if( n > MAX_HOLES )
		error("Poligono com demasiados buracos");
	p.edge = readRing(f);
	p.nHoles = n;
	for( i = 0 ; i < n ; i++ ) {
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

bool insideParcel(Coordinates c, Parcel p)
{
////// FAZER
	return false;
}




/* CARTOGRAPHY -------------------------------------- */
int loadCartography(String fileName, Cartography *cartography)
{
	FILE *f;
	int i;
	f = fopen(fileName, "r");
	if( f == NULL )
		error("Impossivel abrir ficheiro");
	int n = readInt(f);
	if( n > MAX_PARCELS )
		error("Demasiadas parcelas no ficheiro");
	for( i = 0 ; i < n ; i++ ) {
		(*cartography)[i] = readParcel(f);
	}
	fclose(f);
	return n;
}

static int findLast(Cartography cartography, int n, int j, Identification id)
{
	for(  ; j < n ; j++ ) {
		if( !sameIdentification(cartography[j].identification, id, 3) )
			return j-1;
	}
	return n-1;
}

void showCartography(Cartography cartography, int n)
{
	int last;
	Identification header = {"___FREGUESIA___", "___CONCELHO___", "___DISTRITO___"};
	showHeader(header);
	for( int i = 0 ; i < n ; i = last + 1 ) {
		last = findLast(cartography, n, i, cartography[i].identification);
		showParcel(i, cartography[i], last - i + 1);
	}
}


/* INTERPRETER -------------------------------------- */

static bool checkArgs(int arg)
{
	if( arg != -1 )
		return true;
	else {
		printf("ERRO: FALTAM ARGUMENTOS!\n");
		return false;
	}
}

static bool checkPos(int pos, int n)
{
	if( 0 <= pos && pos < n )
		return true;
	else {
		printf("ERRO: POSICAO INEXISTENTE!\n");
		return false;
	}
}

// L
static void commandListCartography(Cartography cartography, int n)
{
	showCartography(cartography, n);
}



static int maxHoleVertexes(Parcel p) {
	if(p.nHoles == 0)
		return 0;
	int max = p.edge.nVertexes;
	for(int i = 0; i < p.nHoles; i++)
	{
		if(max < p.holes[i].nVertexes)
			max = p.holes[i].nVertexes;
	}
	return max;
}

// M pos

/*

Comando Máximo - Dada uma parcela indicada através duma posição no vetor, 
descobre e mostra qual a parcela dessa freguesia que tem mais vértices 
(considerando todos os anéis - exterior e interiores). 
Em caso de parcelas empatadas, mostra qualquer uma delas.

*/
//what the fuck in 'n' ?
static void commandMaximum(int pos, Cartography cartography, int n)
{
	if( !checkArgs(pos) || !checkPos(pos, n) ) 
	{
		// printf("ERRO: POSICAO INEXISTENTE!\n"); there is no need, chechArgs and checkPos have printfs
		return;
	}

		int i = pos;
		Parcel maxParcel = cartography[i];
		i--;
		while(0 <= i) 
		{
			
			if(!sameIdentification(cartography[i].identification, maxParcel.identification,3))
			{
				maxParcel = cartography[i+1];
				break;
			}
			else
			{
				i--;
			}
			
			
		}
		//David, why are you comparing number of wholes with number of vertexes of a ring, there are more nVertexes than nHoles
		int max = maxHoleVertexes(maxParcel);
		max = max < maxParcel.edge.nVertexes ? maxParcel.edge.nVertexes : max;
		int position = i;
		i++;
		while(sameIdentification(maxParcel.identification, cartography[i].identification,3))
		{
			if(max < cartography[i].edge.nVertexes)
			{
				max = cartography[i].edge.nVertexes;
				maxParcel = cartography[i];
				position = i;
			}
			int maxHole = maxHoleVertexes(cartography[i]);
			if(max < maxHole)
			{
				max = maxHole;
				maxParcel = cartography[i];
				position = i;
			}
			i++;
		}
		showParcel(position, maxParcel, max);
}
/**
A function that gives me the max Parcel of a cartography with the b condition, not counting holes
*/

typedef bool BoolFun(Coordinates,Coordinates);

static bool northest(Coordinates c1,Coordinates c2){
	return c1.lat>=c2.lat;
}

static bool southest(Coordinates c1,Coordinates c2){
	return !northest(c1,c2);
}
static bool easthern(Coordinates c1,Coordinates c2){
	return c1.lon>=c2.lon;
}
static bool westest(Coordinates c1,Coordinates c2){
	return !easthern(c1,c2);
}
/** given a parcel, it returns the coordinates of 
 * the northest, southest, eastern and western edges in array given as an argument  */
static void extremeCoordinates(Parcel p1, BoolFun b1, BoolFun b2, BoolFun b3, BoolFun b4, Coordinates res []){
	Ring r = p1.edge;
	Coordinates no = r.vertexes[0];
	Coordinates s,e,w;
	s=e=w=no;
	int len=r.nVertexes;
	
	 for (int i = 1; i < len; i++)
	{
		if(b1(r.vertexes[i],no)){
			no = r.vertexes[i];
		}
		if(b2(r.vertexes[i],s)){
			s = r.vertexes[i];
		} 
		if(b3(r.vertexes[i],e)){
			e = r.vertexes[i];
		} 
		if(b4(r.vertexes[i],w)){
			no = r.vertexes[i];
		}  
	}
	res[0]=no;
	res[1]=s;
	res[2]=e;
	res[3]=w;	
}

// bol: north south east west

// bol: north south east west
void extremeParcel(Cartography carts, int len, BoolFun north, BoolFun south,BoolFun east, BoolFun west ){
	Parcel p1, p2, p3, p4;
	p1 = p2 = p3 = p4 = carts[0];
	int pos1,pos2,pos3,pos4 = 0;
	Coordinates cs [4];
	extremeCoordinates(p1,north, south, east, west, cs);
	Coordinates n = cs[0];
	Coordinates s = cs[1];
	Coordinates e = cs[2];
	Coordinates w = cs[3];

	Coordinates aux;
	for(int i=1;i<len;i++)
	{
		extremeCoordinates(carts[i],north, south, east, west, cs);
		aux = cs[0];
		if(north(aux,n)){ // if c1 the northest
			p1 = carts[i];
			n = aux;
			pos1 = i;
		}
		aux = cs[1];
		if(south(aux,s)){ //compare the southest
			p2 = carts[i];
			s = aux;
			pos2 = i;
		}
		aux = cs[2];
		if(east(aux,e)){ //compare the eastern
			p3 = carts[i];
			e = aux;
			pos3 = i;
		}
		aux = cs[3];
		if(west(aux,w)){ //compare the westest
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
 	
 	if( n == 0 || cartography == NULL)
	{
		printf("ERRO: MAPA VAZIO!\n");
		return;
	} 
	
	
	extremeParcel(cartography,n, northest, southest,easthern,westest); 

}

static void commandResume(int pos, Cartography cartography, int n)
{
	Parcel p = cartography[pos];

	showIdentification(pos, p.identification,3);

		printf("\n%4s %d ","", p.edge.nVertexes);
		showRectangle(p.edge.boundingBox);
		printf("\n");

	if(p.nHoles != 0)
	{
		for (int i = 0; i < p.nHoles; i++)
		{
		printf("%s %d ","", p.holes[i].nVertexes);
		showRectangle(p.holes[i].boundingBox);
		printf("\n");
		}
	}
}

static void commandTravel(double lat, double lon, int pos, Cartography cartography, int n)
{
	if(!checkPos(pos,n)){
		return;
	}
	Coordinates c1 = {lat,lon};
	Parcel p = cartography[pos];
	int len = p.edge.nVertexes;
	Coordinates *cs = p.edge.vertexes;
	double res = haversine(c1,cs[0]);
	double aux;
	
	for (int i = 1; i < len; i++)
	{
		aux = haversine(c1,cs[i]);
		if(aux<res){
			res = aux;
		}
	}
	printf("%f\n",res);	
}

static void commandHowMany(int pos, Cartography cartography, int n)
{
	int towns = 0;
	int counties = 0;
	int districts = 0;

	int i = pos;
	Parcel p = cartography[pos];
	towns++;
	i--;
	while(0 < i)
	{
		if(sameIdentification(p.identification, cartography[i].identification,3))
		{
		towns++;
		p = cartography[i];
		i--;	
		}
		else
		{
			break;
		}
		
		
	}

	p = cartography[pos];
	i = pos;
	i++;
	while(sameIdentification(p.identification, cartography[i].identification,3))
	{
		towns++;
		p = cartography[i];
		i++;
	}

	for(i = 0; i < n; i++)
	{
		if(sameIdentification(cartography[i].identification, cartography[pos].identification,2))
		{
			counties++;
			if(sameIdentification(cartography[i].identification, cartography[pos].identification,1))
			{
				districts++;
			}
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

static int v_strcmp(const void *str1, const void *str2){
	return strcmp(str1,str2);
}


static void commandCounties(Cartography cartography, int n)
{
	StringVector counties;
	strcpy(counties[0], cartography[0].identification.concelho);
	int i = 0;
	int ncounties = 0;
	while(i < n)
	{
		if(strcmp(counties[ncounties], cartography[i].identification.concelho))
		{
			ncounties++;
			strcpy(counties[ncounties], cartography[i].identification.concelho);
		}
		i++;
	}
	ncounties++;
	qsort(counties, ncounties, sizeof(String), v_strcmp);
	showStringVector(counties, ncounties);
}


static void commandDistricts(Cartography cartography, int n)
{
	StringVector districts;
	strcpy(districts[0], cartography[0].identification.distrito);
	int i = 0;
	int ndistricts = 0;
	while(i < n)
	{
		if(strcmp(districts[ndistricts], cartography[i].identification.distrito))
		{
			ndistricts++;
			strcpy(districts[ndistricts], cartography[i].identification.distrito);
		}
		i++;
	}
	ndistricts++;
	qsort(districts, ndistricts, sizeof(String), v_strcmp);
	showStringVector(districts, 5);
}

static bool hasCoords(double lat, double lon, Ring r)
{
	Coordinates cords = coord(lat, lon);
	for(int i = 0; i < r.nVertexes; i++)
	{
		if(sameCoordinates(cords, r.vertexes[i]))
			return true;
	}
	return false;
}


static void commandParcel(double lat, double lon, Cartography cartography, int n)
{
	bool found = false;
	int pos = -1;
	for(int i = 0; i < n && !found; i++)
	{

		if(hasCoords(lat,lon, cartography[i].edge))
		{
				found = true;
				pos = i;
		}
	}
	if(found)
	{
	showIdentification(pos,cartography[pos].identification, 3);
	printf("\n");
	}
	else
	{
		printf("FORA DO MAPA\n");
	}
	
}

static bool vertexInRing(Coordinates c1, Ring r){
	int nOuters = r.nVertexes;
	Coordinates *outers = r.vertexes;
	int i=0;
	while (i<nOuters)
	{
		if(outers[i].lat==c1.lat&&outers[i].lon==c1.lon){
			return true;
		}
		i++;
	}
	return false;	
}
bool adjacentRings(Ring a, Ring b)
{
	int n = a.nVertexes;
	Coordinates *verts = a.vertexes;
	for (int i = 0; i < n; i++)
	{
		if(vertexInRing(verts[i],b)){
			return true;
		}
	}
	return false;
}

bool adjacentParcels(Parcel a, Parcel b)
{
		if (adjacentRings(a.edge,b.edge))
		{
			return true;
		}

		int nHoles = a.nHoles;
		Ring *aHoles = a.holes;

		int bnHoles = b.nHoles;
		Ring *bHoles = b.holes;

		if(nHoles==0 || bnHoles==0){
			return false;
		}
		for (int i = 0; i < nHoles; i++)
		{
			for (int j = 0; j < bnHoles; j++)
			{
				if(adjacentRings(aHoles[i],bHoles[j])){
					return true;
				}
			}
		}
		return false;
}

static int numbers_strcmp(const void *a, const void *b){
	return ( *(int*)a - *(int*)b );
}

static void CommandAdjecent(int pos, Cartography cartography, int n)
{
	if(!checkPos(pos,n)){
		return;
	}
	int ids[10];
	int nps = 0;
	Parcel p = cartography[pos];

	int j = 0;
	
	while (j<n)
	{
		if(j!=pos && adjacentParcels(p,cartography[j])){
			ids[nps++]=j;
		}
		j++;
	}
	if(nps==0){
		printf("NAO HA ADJACENCIAS\n");
		return;
	}
	int haha;
	for (int i = 0; i < nps; i++)
	{
		haha = ids[i];
		showIdentification(haha,cartography[haha].identification,3);
		printf("\n");
	}
}



static int bfs(Cartography carts,int n, int src, int dest){
	int visited [n];
	int dist[n];
	int queue[n];
	int int_max = 2147483647;
	memset(visited,0,sizeof(int)*n);
	memset(dist,0,sizeof(int)*n);
	memset(queue,-1,sizeof(int)*n);

	visited[src] = 1;
	dist[src]=0; //val
	queue[0]=src;
	int elems=1;
	int pop=0;
	int current=src;
	int sol[10];
	for (int i = 0; i < 10; i++)
	{
		sol[i]=+2147483647;
	}
	
	int so=0;
	while (current!=-1&&current<n&&elems<n)
	{	
		current = queue[pop++];
		if(current>=n||current<0){
			break;
		}
		for (int i = 0; i < n; i++)
		{
			if(current != i && adjacentParcels(carts[current],carts[i])){
				if(visited[i]==0){
					if(i!=dest){
						queue[elems++]=i;
						visited[i]=1;
						dist[i]=dist[current]+1;
					}else{
						sol[so++]=dist[current]+1;
					}
				}
			}
		}
	}
	if(so==0){
		return 0;
	}
	int res = sol[0];
	for (int i = 1; i < so; i++)
	{
		if(res>sol[i]){
			res = sol[i];
		}
	}
	return res;
}

static void commandFrontier(int pos1, int pos2, Cartography cartography, int n)
{
	if(!checkPos(pos1,n)||!checkPos(pos2,n)){
		return;
	}
	int res = 0;
	if(pos1==pos2){
		res = 0;
	}else if (adjacentParcels(cartography[pos1],cartography[pos2]))
	{
		res = 1;
	}else{
		res = bfs(cartography,n,pos1,pos2);
	}
	if(res==0){
		printf("NAO HA CAMINHO\n");
	}else{
		printf("%d\n",res);
	}
}

typedef struct Node 
{
	int data;
	struct Node *next;
} Node, *List;

static List newNode(int val, List next)
{
    List n = malloc(sizeof(Node));
    if( n == NULL )
        return NULL;
    n->data = val;
    n->next = next;
    return n;
}
List listPutAtEnd(List l, int val)
{
    if( l == NULL )
        return newNode(val, NULL);
    else {
        List p;
        for( p = l ; p->next != NULL ; p = p->next ); // Stop at the last node
        p->next = newNode(val, NULL);  // Assign to the next of the last node
        return l;
    }
}

static void commandPartition(double distance, Cartography cartography, int n)
{
	
	int result [n];
	int azores [n];
	int madeira [n];
	
	int nAzores = 0;
	int nMads = 0;
	int conts=0;
	int countFinal = 0;
	double gg=0;
	int pass = 1;
	for (int i = 0; i < n; i++)
	{
			if( strcmp("VIANA-DO-CASTELO",cartography[i].identification.distrito)==0||strcmp("EVORA",cartography[i].identification.distrito)==0||strcmp("FARO",cartography[i].identification.distrito)==0){
				result[conts++]=i;
			}else if(strcmp("ARQUIPELAGO-DOS-ACORES",cartography[i].identification.distrito)==0){
				azores[nAzores++]=i;
			}else if (strcmp("ARQUIPELAGO-DA-MADEIRA",cartography[i].identification.distrito)==0)
			{
				madeira[nMads++]=i;
			}

			countFinal = conts;
			for (int i = 0; i < nAzores; i++)
			{
				pass = 1;
				for (int j = 0; j < conts && pass==1; j++)
				{
					if(distance>haversine(cartography[azores[i]].edge.vertexes[0],cartography[result[j]].edge.vertexes[0])){
						pass=0;
					}
				}
				if(pass==1&&conts>0){
					result[countFinal++]=azores[i];
				}
			}
			gg = countFinal-conts;
			
			for (int i = 0; i < nMads; i++)
			{
				pass = 1;
				for (int j = 0; j < conts && pass==1; j++)
				{
					if(distance>haversine(cartography[madeira[i]].edge.vertexes[0],cartography[result[j]].edge.vertexes[0])){
						pass=0;
					}
				}
				if(pass==1&&conts>0){
					result[countFinal++]=madeira[i];
				}
			}
	}

	
	for (int i = 0; i < conts; i++)
	{
		printf("%d ",result[i]);
	} 
	printf("\n");
	for (int i = conts; i < conts+gg; i++)
	{
		printf("%d ",result[i]);
	}
	printf("\n");
	conts = conts+gg;
	for (int i = conts; i < countFinal; i++)
	{
		printf("%d ",result[i]);
	}
}


static void commandCountyHoles(Cartography cartography, int n)
{
	for(int i = 0; i < n; i++){
		if(cartography[i].nHoles != 0)
		{
			showIdentification(i, cartography[i].identification,3);
			printf("\n");
		}
	}
	
}

void interpreter(Cartography cartography, int n)
{
	String commandLine;
	for(;;) {	// ciclo infinito
		printf("> ");
		readLine(commandLine, stdin);
		char command = ' ';
		double arg1 = -1.0, arg2 = -1.0, arg3 = -1.0;
		sscanf(commandLine, "%c %lf %lf %lf", &command, &arg1, &arg2, &arg3);
		// printf("%c %lf %lf %lf\n", command, arg1, arg2, arg3);
		switch( commandLine[0] ) {
			case 'L': case 'l':	// listar
				commandListCartography(cartography, n);
				break;

			case 'M': case 'm':	// maximo
				commandMaximum(arg1, cartography, n);
				break;

			case 'X': case 'x':	// extremos
				commandParcelExtremes(cartography, n);
				break;

			case 'R': case 'r':	// resumo
				commandResume(arg1, cartography, n);
				break;	

			case 'V': case 'v':	// viagem
				commandTravel(arg1, arg2, arg3, cartography, n);
				break;	

			case 'Q': case 'q':	// quantos
				commandHowMany(arg1, cartography, n);
				break;

			case 'C': case 'c':	// concelhos
				commandCounties(cartography, n);
				break;

			case 'D': case 'd':	// distritos
				commandDistricts(cartography, n);
				break;

			case 'P': case 'p':	// parcela
				commandParcel(arg1, arg2, cartography, n);
				break;

			case 'A': case 'a':	// adjacencia
				CommandAdjecent(arg1, cartography, n);
				break;

			case 'F': case 'f':	// fronteira
				commandFrontier(arg1, arg2, cartography, n);
				break;
			
			case 'T': case 't':	// particao
				commandPartition(arg1, cartography, n);
				break;
			
			case 'H': case 'h':	// particao
				commandCountyHoles(cartography, n);
				break;

			case 'Z': case 'z':	// terminar
				printf("Fim de execucao! Volte sempre.\n");
				return;

			default:
				printf("Comando desconhecido: \"%s\"\n", commandLine);
		}
	}
}