
#include "Imagen.h"
#include "Pixel.h"
#include "Matriz.h"
#include "Complejo.h"
#include "Token.h"
#include "Cola.h"

#include <iostream>
#include <sstream>
#include <math.h>

Imagen:: Imagen ()
{
	this->intensidadMax=1;
	this->pixeles.setMatrizNULL();
}

Imagen::Imagen(Matriz<int> intensidadPixeles, size_t sx, size_t sy, int im)
{
	// Si los valores son invalidos, se crea una imagen nula por defecto.
	if(sx < 1 || sy < 1 || im < 1 || im > INTENSIDADMAX_MAX || intensidadPixeles.esVacia()==true)
	{
		this->intensidadMax = 1;
		this->sizeX = 0;
		this->sizeY = 0;
		this->pixeles.setMatrizNULL();
	}

	this->pixeles.setMatrizNULL();
	this->intensidadMax = im;
	this->setPixeles(intensidadPixeles, sx, sy);
}

Imagen::Imagen(const Imagen &i)
{
	this->intensidadMax = i.intensidadMax;
	this->sizeX = i.sizeX;
	this->sizeY = i.sizeY;
	this->pixeles = i.getPixeles();
}

Imagen::~Imagen()
{
}

bool Imagen::setIntensidadMax(int im)
{
	if(im < 1 || im > INTENSIDADMAX_MAX)
		return false;

	this->intensidadMax = im;
	return true;
}

bool Imagen::setPixeles(Matriz<int> intensidadPixeles, size_t sx, size_t sy)
{
	if(sx<1 || sy<1 || intensidadPixeles.esVacia()==true)
		return false;

	if(this->pixeles.esVacia()==false)
		this->pixeles.setMatrizNULL();

	this->sizeX = sx;
	this->sizeY = sy;

	// Se calculan las distancias entre pixeles en el plano complejo
	double distX = 2.0 / (sx - 1);
	double distY = 2.0 / (sy - 1);

	Matriz<Pixel> aux (this->sizeX, this->sizeY);
	
	for(size_t i = 0; i < this->sizeY; i++)
	{
		for(size_t j = 0; j < this->sizeX; j++)
		{
			int intensidad = intensidadPixeles[i][j];

			// Si la intensidad se va de rango, aborto y devuelvo false
			if(intensidad < 0 || intensidad > this->intensidadMax)
				return false;

			Complejo posicion(-1 + j*distX, 1 - i*distY);

			aux [i][j] = Pixel(intensidad, posicion);
		}
	}
	this->pixeles = aux;

	return true;
}

Matriz<Pixel> Imagen::getPixeles() const
{
	return this->pixeles;
}

Matriz<int> Imagen::getIntensidadPixeles() const
{
	if(this->pixeles.esVacia()==true)
		return Matriz<int>();

	Matriz<int> intensidadPixeles (this->sizeX,this->sizeY);
	for(size_t i = 0; i < this->sizeY; i++)
		for(size_t j = 0; j < this->sizeX; j++)
			intensidadPixeles[i][j] = this->pixeles[i][j].getIntensidad();
	return intensidadPixeles;
}

int Imagen::getSizeX() const
{
	return this->sizeX;
}

int Imagen::getSizeY() const
{
	return this->sizeY;
}

int Imagen::getIntensidadMax() const
{
	return this->intensidadMax;
}

Imagen &Imagen::operator = (const Imagen &i)
{
	if(this->pixeles.esVacia()==false)
		this->pixeles.setMatrizNULL();

	this->intensidadMax = i.intensidadMax;
	this->sizeX = i.sizeX;
	this->sizeY = i.sizeY;
	this->pixeles = i.getPixeles();

	return *this;
}

bool Imagen::leerArchivoPgm(istream *iss)
{	
	string line;

    if(!getline(*iss,line))
    	return false;

    if(line.compare("P2")) 
		return false;

    // Se saltean los comentarios
    do {

    if(!getline(*iss,line))
    	return false;

	} while (line[0] == '#');

	istringstream issAux(line);

    long int x, y;
    if(!(issAux >> x) || x < 1)
		return false;
    
    if(!(issAux >> y) || y < 1)
		return false;

	if(!(*iss >> intensidadMax) || intensidadMax < 1 || intensidadMax > INTENSIDADMAX_MAX)
		return false;

	size_t i,j;
	Matriz<int> aux(x,y);
	for(i=0; i < size_t(y); i++)
	{
		for (j=0;j<size_t(x);j++)
			if(!(*iss>>aux[i][j]))	
				return false;		 
	}

	
	// Se chequea si la matriz es más grande de lo que dice, si hay algun numero mas.
	// Si hay algo atrás del último número (sea un espacio, una letra, etc) no es tomado en cuenta.
	if((*iss>>i))
		return false;

	// Se copia la data al objeto imagen.
	this->intensidadMax = intensidadMax;
	if(!this->setPixeles(aux, size_t(x), size_t(y)))
		return false; 

	return true;
}

void Imagen::escribirArchivoPgm(ostream *oss) const
{
	// Si la imagen es nula, no se escribe el archivo.
	if(this->pixeles.esVacia()==true)
		return;

	(*oss) << "P2" << endl
	<< this->sizeX << " " << this->sizeY << endl
	<< this->intensidadMax << endl;

	size_t i, j;
	for(i = 0; i < this->sizeY; i++)
	{
		for(j = 0; j < this->sizeX-1; j++){
			(*oss) << this->pixeles[i][j].getIntensidad() << " ";
		}

		(*oss) << this->pixeles[i][j].getIntensidad() << endl;
	}
}


Imagen Imagen::transformarImagen(Cola<Token> & rpn) const
{
	Imagen dest;

	// Si la imagen no tiene pixeles, se devuelve una imagen con valores nulos
	if(this->pixeles.esVacia()==true)
		return dest;

	dest.setIntensidadMax(this->intensidadMax);
	dest.sizeX=this->sizeX;
	dest.sizeY=this->sizeY;

	double distX = 2.0 / (dest.sizeX - 1);
	double distY = 2.0 / (dest.sizeY - 1);

	Matriz<Pixel> aux(this->sizeX,this->sizeY);
	for(size_t i = 0; i < dest.sizeY; i++)
	{
		for(size_t j = 0; j < dest.sizeX; j++)
		{
			Complejo posDest(-1 + j*distX, 1 - i*distY);
			Complejo posOrig = transformar(rpn, posDest);

			size_t jOrig = round((posOrig.getReal() + 1.0) / distX);
			size_t iOrig = round((1.0 - posOrig.getImag()) / distY);

			int intensidad;
			if(iOrig < 0 || iOrig >= this->sizeY || jOrig < 0 || jOrig >= this->sizeX)
				intensidad = 0;
			else
				intensidad = this->pixeles[iOrig][jOrig].getIntensidad();
			aux[i][j] = Pixel(intensidad, posDest);
		}
	}
	dest.pixeles = aux;
	return dest;
}
