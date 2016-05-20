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
int	** matriz;

/* estructura de datos para entregar argumentos
a las hebras que realizan dilatacion */
struct dat{
	int fila_inicial;
	int cant_filas;
	int id;
};

// función para realizar dilatación
// los resultados son registrados en la 'matriz' global.
void *dilatacion(void *arg){

	struct dat *datos;
	datos = (struct dat*)arg;	

	int cant_filas = datos->cant_filas;
	int fila_inicial = datos->fila_inicial;
	int id = datos->id;
	
	if(DEBUG)
		printf("soy hebra %d, trabajo en %d filas\n",id,cant_filas);		
	
//lectura a evaluar 
	int lsup;
	int linf;
	int lizq;
	int lder;
//posicion puntero
	int centro;

//apertura de archivos
	FILE *pf_in;
	pf_in = fopen(in, "rb");

	if(pf_in==NULL){
		printf("hebra %d, Error al abrir imagen de entrada\n",id);
		printf("hebra %d, Por precaución el programa es abortado.\n",id);
		exit(EXIT_FAILURE);
	}

//lectura
	int lectura;
	int resultado;
	int f,c;
	int aux = 0;
	if(DEBUG)
		printf("hebra %d, fila_inicial %d, posicion_inicial %d.\n",id,fila_inicial,n*fila_inicial);

	for(f = 0; f < cant_filas; f++){	
					
		for(c = 0; c < n; c++){	

			centro = n*fila_inicial+aux;
			fseek(pf_in, centro*sizeof(int), SEEK_SET);//para leer centro
			fread(&lectura, sizeof(int),1,pf_in);

			if((id==0 && f==0) || (f+fila_inicial == n-1) || (c == 0) || (c == n-1)){	//caso primera y última fila del archivo, y primera y ultima columna
				matriz[f+fila_inicial][c] = lectura;
			}
			else{	// dilatacion

				fseek(pf_in, (centro-n)*sizeof(int), SEEK_SET); //posiciono para leer arriba
				fread(&lsup, sizeof(int),1,pf_in);	//leo arriba

				fseek(pf_in, (centro+n)*sizeof(int), SEEK_SET); //posiciono para leer abajo
				fread(&linf, sizeof(int),1,pf_in);	//leo abajo

				fseek(pf_in, (centro-1)*sizeof(int), SEEK_SET); //posiciono para leer izquierda
				fread(&lizq, sizeof(int),1,pf_in);	//leo izquierda

				fseek(pf_in, (centro+1)*sizeof(int), SEEK_SET); //posiciono para leer derecha
				fread(&lder, sizeof(int),1,pf_in);	//leo derecha
				
				if(lsup == 1 || linf == 1 || lder == 1 || lizq == 1 || lectura == 1){	
					resultado = 1;
				}else{
					resultado = 0;
				}
				matriz[f+fila_inicial][c] = resultado;
			}
				
			aux++; //corrimiento del puntero central original
		}
	}
	
	fclose(pf_in);
}


int main (int argc, char* argv[]){
//declaración variables de entrada no globales
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
			case 'i': // imagen entrada .raw	
				in = optarg;			
				if(DEBUG)
					printf("imagen entrada: %s\n",in);
				break;
			case 'O': // imagen salida .raw
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
	if(debug != 0 && debug != 1){
		printf("el valor ingresado para \"debug\" no es válido.\n");
		printf("no se va a realizar debug\n");	
		debug = 0;
	}

//para asignar cant de filas por hebra
	int cant_filas = n/h;
	int resto = n%h;

//para guardar el contenido de salida
	int i,j;
	matriz = (int**)malloc(n*sizeof(int*));
	for(j = 0; j < n; j++){
		matriz[j] = (int*)malloc(n*sizeof(int));			
	}
	

//arreglo de estructuras de datos para pasar a las hebras, y su reserva de mem
	struct dat *argumento;
	argumento = (struct dat*)malloc(sizeof(struct dat)*h);
	
//arreglo de hebras y su reserva de mem
	pthread_t *hebra;
	hebra = (pthread_t*)malloc(sizeof(pthread_t)*h);

//crear hebras
	int rc;
	int fila_inicial = 0;
	for(i = 0; i < h; i++){ //for para crear las h hebras que harán dilatacion.
		argumento[i].id = i+1;
		argumento[i].cant_filas = cant_filas;		
		if(resto > 0){
			argumento[i].cant_filas = cant_filas + 1;
			resto--;
		}		
		argumento[i].fila_inicial = fila_inicial;
		if(DEBUG)
			printf("para h%d, fila ini: %d\n",i+1,fila_inicial);
		fila_inicial = fila_inicial + argumento[i].cant_filas; //calcula fila inicial para la sgte hebra

		rc = pthread_create(&hebra[i], NULL, dilatacion, (void *) &argumento[i]);
		if(rc != 0){
			printf("error al crear h%d, rc = %d\n", i+1, rc);
		}
	}

//esperar finalización de las hebras
	for(i = 0; i < h; i++){
		pthread_join(hebra[i], NULL);
		if(DEBUG)
			printf("Soy padre, hebra %d ha finalizado\n", i+1);
	}

//cerrar archivo que mantuvimos abierto durante la ejecución del programa
	fclose(file_in);
	
//imprimir por consola
	if(debug == 1){
		for(i = 0; i < n; i++){
			for(j = 0; j < n; j++){
				printf("%d",matriz[i][j]);
			}
			printf("\n");
		}	
	}

//padre escribe todo el archivo salida
	file_out = fopen(out,"wb");

	if(file_out==NULL){
		printf("Error al crear imagen de salida\n");
	}

	for(i = 0; i < n; i++){
		for(j = 0; j < n; j++){
			fwrite(&matriz[i][j], sizeof(int),1,file_out);
		}
	}
	fclose(file_out);

	

	return 0;
}

