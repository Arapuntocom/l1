#include <stdio.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>

#define DEBUG 1

//variables globales
char *in;
char *out;
int n;


/* estructura de datos para entregar argumentos
a las hebras que realizan dilatacion */
struct dat{
	int cant_filas;
	int id;
};

// función para realizar dilatación
// los resultados son registrados en la 'matriz' global.
void *dilatacion(void *arg){

	struct dat *datos;
	datos = (struct dat*)arg;	

	int cant_filas = datos->cant_filas;
	int id = datos->id;
	if(DEBUG)
		printf("soy hebra %d, trabajo en %d filas\n",id,cant_filas);		
	
//posiciones a evaluar c/r al ppio del archivo	
	int sup = (id-1)*cant_filas+1;
	int inf = (id+1)*cant_filas+1;
	int izq = id*cant_filas;
	int der = id*cant_filas+2;

//resultado de la evaluacion
	int rsup;
	int rinf;
	int rizq;
	int rder;

//lectura
	int lectura;

	FILE *pf_in;
	fopen(in, "rb");

	FILE *pf_out;
	fopen(out, "wb");

	int resultado;
	int i,j;
	for(i = 0; i < cant_filas; i++){
		
		for(j = 0; j < (n-1); j++){
			
		}
	}
	
	fclose(pf_in);
	fclose(pf_out);

}


int main (int argc, char* argv[]){
//declaración variables de entrada 
//	char *in; 	//imagen de entrada
//	char *out; 	//imagen de salida
//	int n; 		//ancho de imagen
	int h;		//cantidad de hebras
	int debug;	//opción debug

//valores por defecto de variables de entrada
	in = "img_test.raw";
	out = "img_salida.raw";
	n = 12; 	//ancho imagen
	h = 1;		//cantidad de hebras
	debug = 0;	//opcion debug

//otras variables
	FILE *file_in;
	FILE *file_out;

	int option =0;	//opcion getopt
	while( (option = getopt(argc, argv, "i:O:N:H:D:")) != -1){
		switch(option){
			case 'i': // imagen entrada.raw	
				in = optarg;			
				if(DEBUG)
					printf("imagen entrada: %s\n",in);
				break;
			case 'O': // imagen salida.raw
				out = optarg;
				if(DEBUG)
					printf("imagen salida: %s\n",out);
				break;
			case 'N': // ancho imagen
				n = atoi(optarg);
				if(DEBUG)
					printf("ancho imagen: %d\n",n);
				break;
			case 'H': // cant de hebras
				h = atoi(optarg);
				if(DEBUG)
					printf("cantidad hebras: %d\n",h);
				break;
			case 'D': // opción debug, para imprimir imagen por consola
				debug = atoi(optarg);
				if(DEBUG)
					printf("debug: %d\n",debug);
				break;
			default:
				printf("hubo un problema con los argumentos de entrada\n");
				printf("por precaución el programa es abortado.\n");
				exit(EXIT_FAILURE);
		}
	}

// validacion variables de entrada
	//verificando archivo de entrada
	file_in = fopen(in,"rb"); //intentar abrir imagen de entrada, en modo lectura binario.
	if(file_in==NULL){
		printf("Error al abrir imagen de entrada\n");
		printf("Por precaución el programa es abortado.\n");
		exit(EXIT_FAILURE);
	}else{
		
	}
	if(n<0){
		printf("el valor ingresado para \"ancho de imagen\" no es válido.\n");
		n = 3;
		printf("se va a utilizar el valor por defecto: %d\n",n);
	}
	if(h<0){
		printf("el valor ingresado para \"cantidad de hebras\" no es válido.\n");
		h = 1;
		printf("se va a utilizar el valor por defecto: %d\n",h);
	}
	if(debug != 0 || debug != 1){
		printf("el valor ingresado para \"debug\" no es válido.\n");
		printf("no se va a realizar debug\n");	
	}

//para asignar cant de filas por hebra
	int cant_filas = n/h;
	int resto = n%h;

//arreglo de estructuras de datos para pasar a las hebras, y reserva de mem
	struct dat *argumento;
	argumento = (struct dat*)malloc(sizeof(struct dat)*h);
	
//arreglo de hebras y reserva de mem
	pthread_t *hebra;
	hebra = (pthread_t*)malloc(sizeof(pthread_t)*h);

//crear hebras
	int i;
	int rc;
	for(i = 0; i < h; i++){ //for para crear las n hebras que harán sum.
		argumento[i].id = i+1;
		argumento[i].cant_filas = cant_filas;		
		if(resto > 0){
			argumento[i].cant_filas = cant_filas + 1;
			resto--;
		}
		rc = pthread_create(&hebra[i], NULL, dilatacion, (void *) &argumento[i]);
		if(rc != 0){
			printf("error al crear h%d, rc = %d\n", i+1, rc);
		}
	}

//esperar finalización de las hebras
	for(i = 0; i < h; i++){
		pthread_join(hebra[i], NULL);
		printf("Soy padre, hebra %d ha finalizado\n", i);
	}
	
	
	
	fclose(file_in);
	return 0;
}

