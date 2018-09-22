/*
Verónica Montserrat Martínez Lona

a ) Desribir una circunferencia con centro y radio
b ) Describir una circunferencia con 3 puntos dados (en cualquier posicion de la imagen)
c )  Describir una circunferencia con 3 puntos dados (solo pixeles negros)

Nota: Todo lo referente al manejo de imagenes:
"Biblioteca Basica de Funciones
Autor: Dr. Carlos Hugo Garcia Capulin
Ver 1.2"
*/

#include<stdio.h>
#include<stdlib.h>
#include<math.h>

#define PI (3.141592653589793)
// ____________________ Tipos de datos a utilizar
typedef unsigned char  		byte;    	// Tipo de dato de 1 byte
typedef unsigned short int 	word;    // Tipo de dato de 2 bytes
typedef unsigned long  int 	dword;   // Tipo de dato de 4 bytes

// ____________________ Variables de apoyo
const word NUMBER_PARTICLES = 5;
const word NUMBER_PARAMETERS = 6;
const word NUMBER_ITERATIONS = 3;
const float C1 = 2; //por teoria toman este valor
const float C2 = 2;
const float Vmin = -1.0;
const float Vmax = 1.0;

// gcIMG: Nos permite manejar una imagen
typedef struct{
	byte    id[2];  // Identificador de fila BMP
	word    offset; // Offset al principio de la imagen
	word    ancho;  // Columnas de la imagen
	word    alto;   // Filas de la imagen
	byte    bpp;    // Bits de color por pixel
  int     size;   // Tamaño de la imagen
  byte    *head;  // Puntero al encabezado
	float   *imx;   // Puntero al inicio de la imagen
}gcIMG;

//Particle definition
typedef struct {
  word *Xi;  //Actual Position
  word *Vi;  //Actual Speed
  word *Pi;  //Best Position
  word XFit; //Fitness value
  word PFit; //Best position fitness value
} PARTICLE;

//Swarm definition
typedef struct {
  word     numParticles;  //Number of particles of the swarm
  word     numParameters; //Number of parameters of the equation
  word     fitGBest;      //Value of the best particle
  word     idGbest;       //id of the best particle
  word     infLx;         // x Limits of the image
  word     supLx;
  word     infLy;         // y Limits of the image
  word     supLy;
  float    Vmin;
  float    Vmax;
  float    c1; //Random constant
  float    c2; //Random constant
  PARTICLE *particle;     //Swarm
} SWARM;

// ____________________ Function prototype
// Image manager
gcIMG* 	gcGetImgBmp(char *ruta);
void 	gcPutImgBmp(char *ruta, gcIMG *img);
gcIMG*	gcNewImg(int ancho,int alto);
void 	gcFreeImg (gcIMG *img);
//PSO
SWARM *CreateSwarm(const unsigned int numParticles, const unsigned int numParameters);
void CreateParticle(SWARM *pSwarm, const word i, const word numParameters);
void InitSwarm(SWARM *pSwarm, const float Vmin, const float Vmax, const word c1, const word c2, const char method);
void InitBest(SWARM *pSwarm);
// ___ For the 3-point equation
word ThreePointInitPosition(const word j, const word infLx, const word supLx, const word infLy, const word supLy);
void ThreePoint(SWARM *pSwarm, const gcIMG *image);
void ThreePointBlack(SWARM *pSwarm, const gcIMG *image);
void DefineEquation(const PARTICLE *pParticle, word *xc, word *yc, word *r, const word *supLx, const word *supLy);
void BuildMatrix(const PARTICLE *pParticle, word matrix[4][3]);
word *AuxiliarMatrix(const word matrix[4][3], const int exc);
float ComputeDeterminant(const word *matrix);
//
void EvaluateSwarm(PARTICLE *pParticle, const gcIMG *image, const word xc, const word yc, const word r);
void UpdateSpeed(SWARM *pSwarm);
void UpdatePosition(SWARM *pSwarm);
void UpdateBest(SWARM *pSwarm);
void ShowParticle(const PARTICLE *pParticle, const word i, const word numParameters);
void ShowSwarm(const SWARM *pSwarm);
void DeleteSwarm(SWARM *pSwarm);

int main (int argc, char** argv)
{
  if (argc != 2 )
  {
    printf("Ruta de imagen no ingresada o ingresada incorrectamente.");
    return -1;
  }

	srand(time(NULL));
	//Declarar un Puntero a imagen
  gcIMG *Img;
	//Leer la imagen desde una ruta dada por el usuario
  Img = gcGetImgBmp(argv[1]);

  //Obtiene sus dimensiones
  printf("\n El ancho es: %i",Img -> ancho);
  printf("\n El alto es: %i",Img -> alto);
  printf("\n El tamanio es: %i",Img -> size);
  printf("\n\n");

  SWARM *expSwarm;
  word it = 0;
  word maxIt = NUMBER_ITERATIONS;
  //Crear memoria para el enjambre
  expSwarm = CreateSwarm(NUMBER_PARTICLES, NUMBER_PARAMETERS);
  //Inicializar posicones de las partículas ente los límite del espacio de búsqueda del problema
  expSwarm -> infLx = 0;
  expSwarm -> supLx = Img -> ancho;
  expSwarm -> infLy = 0;
  expSwarm -> supLy = Img -> alto;
  InitSwarm(expSwarm, Vmin, Vmax, C1, C2, 'b'); //case b
	//printf("expSwarm = (%u, %u)\n" expSwarm -> particle[i].Xi[0], expSwarm -> particle[i].Xi[1]);
	//Iniciar el fitness de cada partícula e identificar el fitness de la mejor global
  InitBest(expSwarm);
  ShowSwarm(expSwarm);

  while(it < maxIt)// && (360 - expSwarm -> particle[expSwarm -> idGbest].PFit) > 5)
  {
    //Evaluar las partículas del enjambre en la ecuacion
    ThreePoint(expSwarm, Img);
    /*
    UpdateSpeed(expSwarm);
    UpdatePosition(expSwarm);
    UpdateBest(expSwarm);
		*/
    //Mostrar todas las partículas del enjambre
    ShowSwarm(expSwarm);
    //printf("idGbest: %d \n", expSwarm -> idGbest);
    printf("it = %d ------------------------------------------------\n", it);
    it++;

  }

  //Libera la Imagen utilizada
  gcFreeImg(Img);
	//Liberar la memoria del enjambre
  DeleteSwarm(expSwarm);

  return 0;
}

/*           FUNCIONES BASE DE PROCESAMIENTO DE IMAGENES          */
/********************Funcion Uno ***********************************
 Garcia Capulin Get Image Bmp
 gcGetImgBmp()
 Funcion que lee un archivo bmp y lo coloca en memoria para su Proc.
*/
gcIMG* gcGetImgBmp(char *ruta)
{ gcIMG *img;
  FILE *file;
  int  i,j,a,ar;

// Abrir Archivo de entrada
  if  ( (file = fopen(ruta,"rb"))==NULL )
      { printf(" Error al Abrir Archivo \n");
	exit(1);
	}
// Asignar memoria para la estructura gcIMG
  if  ( (img = (gcIMG *) calloc(1,sizeof(gcIMG)) ) == NULL)
      { printf("Error al reservar memoria para gcIMG \n");
        exit (1);
        }

  fread(img->id,2,1,file);      // Lee 2 bytes del identificador
  fseek(file,10,SEEK_SET);      // Se posiciona en Data offset
  fread(&img->offset,2,1,file); // Lee el offset de la Imagen
  fseek(file,18,SEEK_SET);      // Se posiciona en Width
  fread(&img->ancho,2,1,file);  // Lee el ancho de la Imagen
  fseek(file,22,SEEK_SET);      // Se posiciona en Height
  fread(&img->alto,2,1,file);   // Lee el alto de la Imagen
  fseek(file,28,SEEK_SET);      // Se posiciona en Bits p/pixel
  fread(&img->bpp,1,1,file);    // Lee los Bpp
  fseek(file,34,SEEK_SET);      // Se posiciona en Size
  fread(&img->size,4,1,file);   // Lee el tamaño de la Imagen */

// Comprobar archivo valido
  if  ( (img->id[0]!='B')||(img->id[1]!='M') )
      { printf("Archivo de Formato No Valido \n");
        exit (1);
        }

// Asignar memoria para el encabezado
  if ( (img->head = (unsigned char *) malloc(img->offset)) == NULL )
     { printf("Error al reservar memoria para el encabezado \n");
       exit (1);
       }

// Asignar memoria para la imagen real
  if ( (img->imx =(float *)calloc(img->ancho*img->alto,sizeof(float))) == NULL )
     { printf("Error al reservar memoria para la imagen \n");
       exit (1);
       }

// Lectura del encabezado
  rewind(file);
  fread(img->head,1078,1,file);

// Lectura de la imagen
  a=img->ancho;
  ar=img->size/img->alto;               //calcula el ancho real
  fseek(file,img->offset,SEEK_SET);     // Se posiciona al inicio de la imagen
  for (i=0; i<img->alto; i++)
    { for(j=0; j<img->ancho; j++)
      img->imx[i*a+j]=(float)fgetc(file);
      if(ar!=a) for(j=0;j<ar-a;j++) fgetc(file);  // Si el ancho es mayor
      }                                           // brinca esos datos
  fclose(file);
  img->size=img->ancho*img->alto;       //Asigna el Tamaño Real de la Imagen
  return img;
}

/******************** Funcion Dos ***********************************
 Garcia Capulin Put Image Bmp
 gcPutImgBmp()
  Funcion que envia a un archivo bmp una imagen en memoria.
*/

void gcPutImgBmp(char *ruta, gcIMG *img)
{ FILE *file;
  int aux,zero=0,i,j,offset,Newancho;

// Crear un Archivo nuevo
  if ((file = fopen(ruta,"w+b")) == NULL)
        { printf("\nError abriendo el archivo \n");
          exit(1);
          }
//Checar si el ancho es multiplo de 4
  offset=img->ancho%4;
  if (offset) Newancho=img->ancho+(4-offset); //Si no hacerlo multiplo
     else     Newancho=img->ancho;           // Si, si mantenerlo

// Checar el encabezado
  if (img->head) { img->size=(Newancho*img->alto); //Modificar el bitmap size
                   fwrite(img->head,1078,1,file);
                   }
// Generar encabezado:
     else {
            fputc('B',file); fputc('M',file);   // Escribe BMP Identificador
	    aux = Newancho * img->alto + 1078;
	    fwrite(&aux,4,1,file);              // Escribe File Size
	    fwrite(&zero,4,1,file);             // Escribe Word Reserved
            aux=1078;
	    fwrite(&aux,4,1,file);              // Escribe Data Offset
	    // Image Header
	    aux=40;
	    fwrite(&aux,4,1,file);              // Escribe Header Size
	    aux=img->ancho;
	    fwrite(&aux,4,1,file);              // Escribe Width
	    aux=img->alto;
	    fwrite(&aux,4,1,file);              // Escribe Height
	    aux=1;
	    fwrite(&aux,2,1,file);              // Escribe Planes
	    aux=8;
	    fwrite(&aux,2,1,file);              // Escribe Bits p/pixel
	    aux=0;
	    fwrite(&aux,4,1,file);              // Escribe Compression
            aux=(Newancho*img->alto);
	    fwrite(&aux,4,1,file);              // Escribe Bitmap Size
	    aux=0;
	    fwrite(&aux,4,1,file);              // Escribe HResolution
            fwrite(&aux,4,1,file);              // Escribe VResolution
            aux=256;
            fwrite(&aux,4,1,file);              // Escirbe Colors used
            aux=0;
            fwrite(&aux,4,1,file);              // Escirbe Important Colors

// Escritura de la paleta
   	    for (aux=0; aux<256; aux++)
                { for (i=0; i<3; i++) fwrite(&aux,1,1,file);
		  fwrite(&zero,1,1,file);
		  }
	  }
// Escritura del mapa de bits
  aux=img->ancho;
  for(i=0;i<img->alto;i++)
      for(j=0;j<Newancho;j++)
        { if(j>aux-1) fputc(0,file);
          else fputc((unsigned char)img->imx[i*aux+j],file);
         }
  fclose(file);
}

/******************** Funcion Tres ***********************************
 Garcia Capulin New Image
 gcNewImg()
  Funcion que genera una matriz vacia de una imagen en memoria.
*/

gcIMG *gcNewImg(int ancho,int alto)
{
  gcIMG *img;
  int i;

  if (( img = (gcIMG *) calloc(1,sizeof(gcIMG)) ) == NULL)
        { printf("Error al reservar memoria para gcIMG\n");
          exit (1);
          }
  img->ancho = ancho;
  img->alto  = alto;
  img->size  = ancho*alto;
  if (( img->imx = (float *) calloc(img->size,sizeof(float)) ) == NULL)
        { printf("Error al reservar memoria para la Imagen \n");
          exit (1);
          }
  img->head = NULL;
  return img;
}

// Funcion que libera la memoria de una imagen.
 void gcFreeImg (gcIMG *img)
 { free(img->head);
   free(img->imx);
   free(img);
  }

//_________________________________________________________________PSO
// ____________________ Creates the memory needed to handle the swarm and initialize variables.
SWARM *CreateSwarm(const unsigned int numParticles, const unsigned int numParameters)
{
  SWARM *pSwarm;
  //Assign memory to the swarm structure
  pSwarm = (SWARM*)malloc(sizeof(SWARM));
  if (pSwarm == NULL)
  {
    printf("Error al asignar memoria a la estructura enjambre.\n");
    return -1;
  }
  //Initialize the structure
  //Assign particle number
  pSwarm -> numParticles = numParticles;
  //Assign number of parameters
  pSwarm -> numParameters = numParameters;
  //Assign memory to particles
  pSwarm -> particle = (PARTICLE*)malloc(numParticles*sizeof(PARTICLE));
  if(pSwarm -> particle == NULL)
  {
    printf("Error al asignar memoria a las particulas del enjambre.\n");
    return -1;
  }
  //Assign memory to each particle
  for (word i = 0; i < numParticles; i++)
  {
      CreateParticle(pSwarm, i, numParameters);
  }

  return pSwarm;
}

// ____________________  Allocate memory to particles and their elements:
// ____________________  position, speed and bestFitness
void CreateParticle(SWARM *pSwarm, const word i, const word numParameters)
{
    pSwarm -> particle[i].Xi = (word*)malloc(numParameters*sizeof(word));
    pSwarm -> particle[i].Vi = (word*)malloc(numParameters*sizeof(word));
    pSwarm -> particle[i].Pi = (word*)malloc(numParameters*sizeof(word));
    if(pSwarm -> particle[i].Xi == NULL || pSwarm -> particle[i].Vi == NULL || pSwarm -> particle[i].Pi == NULL)
    {
      printf("Error al asignar memoria a las particulas.\n");
      return -1;
    }
    return;
}

// ____________________ Initialize the variables of the particles of the swarm
//_____________________ Position, Speed, FitnessValue, best position
void InitSwarm(SWARM *pSwarm, const float Vmin, const float Vmax, const word c1, const word c2, const char method)
{
  word i, j;
  double r;

  //Swarm constant variables
  pSwarm -> c1 = c1;
  pSwarm -> c2 = c2;

  //Swarm speed limits
  pSwarm -> Vmin = Vmin;
  pSwarm -> Vmax = Vmax;

  //For all particles
  for(i = 0; i < pSwarm -> numParticles; i++)
  {
    //For all parameters
    for(j = 0; j < pSwarm -> numParameters; j++)
    {
      //Para el caso b)
      switch (method)
      {
        case 'b':
          r = ThreePointInitPosition(j, pSwarm -> infLx, pSwarm -> supLx, pSwarm -> infLy, pSwarm -> supLy);
          break;
        default:
          break;
      }
      //Random position
      pSwarm -> particle[i].Xi[j] = r;
      //Speed = 0, still do not start to move
      pSwarm -> particle[i].Vi[j] = 0;
      //Random weigth
      pSwarm -> particle[i].Pi[j] = 0;
    }
    pSwarm -> particle[i].XFit = 0;
    pSwarm -> particle[i].PFit = 0;
  }
  return;
}

// ____________________ Controla la posicion que pueden tomar los 3 puntos iniciales
// ____________________ (Solo para la ecuacion de 3-puntos)
word ThreePointInitPosition(const word j, const word infLx, const word supLx, const word infLy, const word supLy)
{
	//printf("ThreePointInitPosition\n\n");
  word r;
  //Los puntos deben caer dentro de la imagen
  if(j % 2) //En x
  {
    r = (1.0 * rand() / RAND_MAX) * (supLx - infLx) + infLx;
  }
  else //en y
  {
    r = (1.0 * rand() / RAND_MAX) * (supLy - infLy) + infLy;
  }

  return r;
}

// ____________________ Init the best positions
// ____________________ XFit, PFit, fitGBest
void InitBest(SWARM *pSwarm)
{
  word i, best = 0;

  for(i = 0; i < pSwarm -> numParticles; i++)
  {
    //In the initialisation of the best weight is directly the weight of the particle, since there is no more in history
    pSwarm -> particle[i].PFit = pSwarm -> particle[i].XFit;
    if(pSwarm -> particle[i].XFit > best)
    {
        //Compare the weights of the swarm particles to know which one is the best
        pSwarm -> idGbest  = i;
        best = pSwarm -> particle[i].PFit;
        pSwarm -> fitGBest = pSwarm -> particle[i].PFit;
    }
  }
  return;
}

// ____________________ Resolvemos para la ecuacion de 3-puntos
// ____________________ Metodo: Las particulas pueden tomar cualquier posicion en la imagen
// ____________________ (Solo para la ecuacion de 3-puntos)
void ThreePoint(SWARM *pSwarm, const gcIMG *image)
{
	printf("ThreePoint\n");
  word xc, yc, r, i, j, x, y;
  word isBlack = 0;

  for(i = 0; i < pSwarm -> numParticles; i++)
  {
    //Para todos los parametros
    for(j = 0; j < pSwarm -> numParameters; j += 2)
    {
      //Revisar que todos los puntos sean negros. Si no es negro no hay circunferencia que buscar
      x = pSwarm -> particle[i].Xi[j];
      y = pSwarm -> particle[i].Xi[j + 1];

      if(image -> imx[x * image -> ancho + y] == 0)
      {
				isBlack ++;
      }
      else
      {
				break;
      }
    }
    if(isBlack != 3)
    {
      pSwarm->particle[i].XFit = 0;
      continue;
    }

		printf("ThreePoint1\n");
    //Definimos los parámetros r, xc, yc para cada partícula (conjunto de 3 puntos)
    DefineEquation(pSwarm -> particle + i, &xc, &yc, &r, &pSwarm -> supLx, &pSwarm -> supLy);
		printf("ThreePoint2\n");
		EvaluateSwarm(pSwarm -> particle + i, image, xc, yc, r);
		printf("ThreePoint3\n");
  }
  return;
}

// ____________________ Resolvemos para la ecuacion de 3-puntos
// ____________________ Metodo: Las particulas  solo pueden tomar posiciones negras en la imagen
// ____________________ (Solo para la ecuacion de 3-puntos)
void ThreePointBlack(SWARM *pSwarm, const gcIMG *image)
{
  word xc, yc, r, i, j, x, y;

  for(i = 0; i < pSwarm -> numParticles; i++)
  {
    //Definimos los parámetros r, xc, yc para cada partícula (conjunto de 3 puntos)
    DefineEquation(pSwarm -> particle + i, &xc, &yc, &r, pSwarm -> supLx, pSwarm -> supLy);
    EvaluateSwarm(pSwarm, image, xc, yc, r);
  }
  return;
}

// ____________________ Define the circle equation
// ____________________ Calculate parameters of the equation:
// ____________________ radius and center position
// ____________________ (Solo para la ecuacion de 3-puntos)
void DefineEquation(const PARTICLE *pParticle, word *xc, word *yc, word *r, const word *supLx, const word *supLy)
{
	printf("DefineEquation\n");
  word matrix[4][3];
  //Build the matrix of the equations system
  BuildMatrix(pParticle, matrix);
	printf("1");
	//Resolver el sistema de ecuaciones mediante determiantes
  float detP, detD, detE, detF;
  detP = ComputeDeterminant(AuxiliarMatrix(matrix, -1));
	detD = ComputeDeterminant(AuxiliarMatrix(matrix,  0));
	detE = ComputeDeterminant(AuxiliarMatrix(matrix,  1));

  //Calcular las variables de la ecuación
  float D, E, F;
  D = detD / detP;
  E = detE / detP;

  //Centro
  *xc = (int) D / -2;
	*yc = (int) E / -2;

	// Compute radio
	//r = sqrt(x2 - x1 '2' + y2-y1'2')
	int dx = pParticle -> Xi[0] - *xc;
	int dy = pParticle -> Xi[1] - *yc;
  r = (int)sqrt(dx * dx + dy * dy);

	printf("xc = %u, yc = %u, r = %u\n", *xc, *yc, *r);
  return;
}

// ____________________ Build the matriz of the equation system
// ____________________ (Solo para la ecuacion de 3-puntos)
void BuildMatrix(const PARTICLE *pParticle, word matrix[4][3])
{
	printf("BuildMatrix\n");
	printf("BuildMatrix\n");
	printf("BuildMatrix\n");
	printf("BuildMatrix\n");
	printf("BuildMatrix\n");
	printf("BuildMatrix\n");
	printf("Hola");



}

//  ____________________ Construye las matrices 3x3
// ____________________ para la resolución del sistema de ecuaciones por determinantes
// ____________________ (Solo para la ecuacion de 3-puntos)
word *AuxiliarMatrix(const word matrix[4][3], const int exc)
{
  word *auxMatrix;
  word i, j;

  auxMatrix = (word*) malloc (9 * sizeof(word));

  for(i = 0 ; i < 3 ; i++)
  {
    for(j = 0; j < 3; j++)
    {
      auxMatrix[i * 3 + j] = matrix[ i == exc ? 3 : i ][j];
    }
  }
  return auxMatrix;
}

//Compute a simple 3x3 determinant (This is for the equations resolution)
// ____________________ (Solo para la ecuacion de 3-puntos)
float ComputeDeterminant(const word *matrix)
{
  // det (A) = aei + bfg + cdh - afh - bdi - ceg.
  float a = matrix[0], b = matrix[1], c = matrix[2];
  float d = matrix[3], e = matrix[4], f = matrix[5];
  float g = matrix[6], h = matrix[7], i = matrix [8];

  double det = (a * e * i + b * f * g + c * d *h);
  det -= (a * f * h + b * d * i + c * e * g);

  return det;
}

 // ____________________ Evaluamos cada particula del enjambre en la ecuacion
void EvaluateSwarm(PARTICLE *pParticle, const gcIMG *image, const word xc, const word yc, const word r)
{
	printf("EvaluateSwarm1\n");
	printf("xc = %u, y = %u, r = %u\n", xc, yc, r);
  float angulo;
  word x, y;

  //Evaluamos las ecuaciones
  // x = r cos(a) + xc
  // y = r sen(a) + yc
  // Entre mas cerca este XFit de 360 mejor sera como solucion
	printf("EvaluateSwarm2\n");
  for(angulo = 0; angulo < 2 * PI; angulo += PI / 180)
  {
    x = (int) r * cos(angulo) + xc;
    y = (int) r * sin(angulo) + yc;
    //Si el pixel es negro aumenta el fitness
    if(image->imx[x * image -> ancho + y] == 0)
    {
      pParticle -> XFit++;
    }
    else continue;
  }
	return;
}

// ____________________ Calculamos la velocidad por medio de la formula que nos da el algoritmo PSO
void UpdateSpeed(SWARM *pSwarm)
{
  word i, j;
  float y1, y2, aux;

  //Para todas las partículas
  for(i = 0; i < pSwarm -> numParticles; i++)
  {
    //para todos los parámetros
    for(j = 0; j < pSwarm -> numParameters; j++)
    {
      //Variables aleatorias
      y1 = (1.0 * rand()/RAND_MAX);
      y2 = (1.0 * rand()/RAND_MAX);
      //Càlculo de la formula que marca el algoritmo
      aux = pSwarm -> particle[i].Vi[j] + pSwarm -> c1 * y1 * (pSwarm -> particle[i].Pi[j] - pSwarm -> particle[i].Xi[j]) +

                                     pSwarm -> c2 * y2 * (pSwarm -> particle[pSwarm -> idGbest].Pi[j] - pSwarm -> particle[i].Xi[j]);

//Controlamos los límites de velocidad para que las partículas no se pierdan tan facilmente

      if(aux > pSwarm -> Vmax)
      {
        pSwarm -> particle[i].Vi[j] = pSwarm -> Vmax;
        continue;
      }
      if(aux < pSwarm -> Vmin)
      {
        pSwarm -> particle[i].Vi[j] = pSwarm -> Vmin;
        continue;
      }

      pSwarm -> particle[i].Vi[j] = aux;
    }
  }

  return;
}

// ____________________ Calculamos la nueva posicion de las particulas, despues de moverse
// ____________________ Los límites son para no salir de la imagen
void UpdatePosition(SWARM *pSwarm)
{
  word i, j;
  word position;

  //para todas las particulas
  for(i = 0; i < pSwarm -> numParticles; i++)
  {
    //para todos los parametros
    for(j = 0; j < pSwarm -> numParameters; j++)
    {
      //posicion actual mas la velocidad
      position = pSwarm -> particle[i].Xi[j] + pSwarm -> particle[i].Vi[j];
      //Si se sale de los limites en x
      if(j % 2 && position > pSwarm -> supLx) //Para x
      {
        continue;
      }
      //Si sale de los limites en y
      else if(!(j % 2) && position > pSwarm -> supLy) //Para y
      {
        continue;
      }
      pSwarm -> particle[i].Xi[j] += pSwarm -> particle[i].Vi[j];
    }
  }

  return;
}

// ____________________ Comparamos las posibles soluciones
// ____________________ para identificar si hay una nueva mejor solucion
void UpdateBest(SWARM *pSwarm)
{
  word i, j;
  //Peso de la mejor partícula del enjambre
  float best = pSwarm -> particle[pSwarm -> idGbest].PFit;

  //Para todas las particulas
  for(i = 0; i < pSwarm -> numParticles; i++)
  {
    //Si el peso de actual es mayor que el del historico de esa particula
    if(pSwarm -> particle[i].XFit > pSwarm -> particle[i].PFit)
    {
      //Para todos los parametros
      for(j = 0; j < pSwarm -> numParameters; j++)
      {
        //Se actualizan los pesos optimos
        pSwarm -> particle[i].Pi[j] = pSwarm -> particle[i].Xi[j];
        pSwarm -> particle[i].PFit = pSwarm -> particle[i].XFit;
      }
    }
    //Si el mejor peso actual de la particula es mejor que el de la mejor del ejambre
    if(pSwarm -> particle[i].PFit > best)
    {
      //Se actualiza el id y peso del mejor
      pSwarm -> idGbest = i;
      best = pSwarm -> particle[i].PFit;
    }
  }

  return;
}

// ____________________ Mostramos los parametros de interes de las particulas
void ShowParticle(const PARTICLE *pParticle, const word i, const word numParameters)
{
  word j;

  printf("\nX%u : ", i);
  for(j = 0; j < numParameters; j++)
  {
    printf("%3u  ", pParticle -> Xi[j]);
  }
  printf("\nV%u : ", i);
  for(j = 0; j < numParameters; j++)
  {
    printf("%u  ", pParticle -> Vi[j]);
  }
  printf("\nP%u : ", i);
  for(j = 0; j < numParameters; j++)
  {
    printf("%3u  ", pParticle -> Pi[j]);
  }
  printf("\nXFit%u : ", i);
  printf("%u   ", pParticle -> XFit);

  printf("PFit%u : ", i);
  printf("%u\n", pParticle -> PFit);

  return;
}

// ____________________ Mostramos las particulas que conforman el ejambre
void ShowSwarm(const SWARM *pSwarm)
{
  unsigned int i;
  //Para todas las partículas
  for(i = 0; i < pSwarm -> numParticles; i++)
  {
    ShowParticle(pSwarm -> particle + i, i, pSwarm -> numParameters);
  }

  return;
}

// ____________________ Liberamos la memoria asignada al enjambre
void DeleteSwarm(SWARM *pSwarm)
{
  unsigned int i;

  for (i = 0; i < pSwarm -> numParticles; i++)
  {
    free(pSwarm -> particle[i].Xi);
    free(pSwarm -> particle[i].Vi);
    free(pSwarm -> particle[i].Pi);
  }

  free(pSwarm);

  return;
}
