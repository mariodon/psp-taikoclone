#include "oslib.h"

/*
	Cette fonction est tr�s lente sous windows. Sans les buffers auxiliaires, voici les op�rations
	qu'elle effectue:
	Sauvegarde le contenu du drawbuffer courant vers l'image du drawbuffer courant
	Code sp�cial pour OSL_DEFAULT/OSL_SECONDARY_BUFFER
	Ecrit le contenu du nouveau drawbuffer sur la carte graphique
*/
void oslSetDrawBuffer(OSL_IMAGE *img)		{
#ifdef PSP
	osl_curBuf = img;
	sceGuDrawBuffer(img->pixelFormat, oslRemoveVramPrefixPtr(img->data), img->sysSizeX);
#else
	//M�me image? Aucun int�r�t
	if (img == osl_curBuf)
		return;
	//On doit d�sactiver (temporairement) le texturage 2D
	emuConfigure2DTransfer(1);
	//Lit les donn�es du backbuffer dans l'image utilis�e jusque l� comme drawbuffer
	if ((osl_curBuf != OSL_SECONDARY_BUFFER && osl_curBuf != OSL_DEFAULT_BUFFER) || (img != OSL_DEFAULT_BUFFER && img != OSL_SECONDARY_BUFFER))
		emuGlReadPixels(0, 272-osl_curBuf->sizeY, osl_curBuf->sysSizeX, osl_curBuf->sizeY, GL_RGBA, osl_curBuf->pixelFormat, osl_curBuf->data);
	//OPENGL SPECIFIC!!!!
	if (img == OSL_SECONDARY_BUFFER || OSL_DEFAULT_BUFFER->data == OSL_SECONDARY_BUFFER->data)		{
		glReadBuffer(GL_FRONT);
		glDrawBuffer(GL_FRONT);
	}
	else		{
		glReadBuffer(GL_BACK);
		glDrawBuffer(GL_BACK);
	}
	//Ecrit la nouvelle image sur le backbuffer
	if ((osl_curBuf != OSL_SECONDARY_BUFFER && osl_curBuf != OSL_DEFAULT_BUFFER) || (img != OSL_DEFAULT_BUFFER && img != OSL_SECONDARY_BUFFER))		{
		glRasterPos2i(0, 0);
		glPixelZoom(1, -1);
		glDrawPixels(img->sysSizeX, img->sizeY, GL_RGBA, emu_pixelPhysFormats[img->pixelFormat], img->data);
	}
	osl_curBuf = img;
	//R�active si jamais
	emuConfigure2DTransfer(0);
#endif
	oslSetScreenClipping(0, 0, img->sizeX, img->sizeY);
}

