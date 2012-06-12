#include "oslib.h"

/*
	Fonctions sp�ciales PC
	Permet d'acc�der en soft � une image s�lectionn�e comme drawbuffer ou texture
*/
void oslLockImage(OSL_IMAGE *img)
{
	//Drawbuffer courant?
	if (img == osl_curBuf)			{
		oslSyncDrawing();
		//Avec OpenGL, il faut copier le contenu du drawbuffer sur l'image (vu qu'on va l'utiliser)
		#ifndef PSP
			//On doit d�sactiver (temporairement) le texturage 2D
			int bTextureEnabled = glIsEnabled(GL_TEXTURE_2D), bBlendingEnabled = glIsEnabled(GL_BLEND);
			glDisable(GL_TEXTURE_2D);
			glDisable(GL_BLEND);
			//Lit les donn�es du backbuffer dans l'image utilis�e jusque l� comme drawbuffer
			emuGlReadPixels(0, 272-osl_curBuf->sizeY, osl_curBuf->sysSizeX, osl_curBuf->sizeY, GL_RGBA, osl_curBuf->pixelFormat, osl_curBuf->data);
			//R�active si jamais
			if (bTextureEnabled)
				glEnable(GL_TEXTURE_2D);
			if (bBlendingEnabled)
				glEnable(GL_BLEND);
		#endif
	}
}

//A appeler une fois termin�
void oslUnlockImage(OSL_IMAGE *img)
{
//	if (img->location == OSL_IN_RAM)
		oslUncacheImage(img);
	//Texture courante?
	if (img->data == osl_curTexture)
		osl_curTexture = NULL;
	//Avec OpenGL, on recopie le contenu de l'image (modifi�e) vers le framebuffer
	#ifndef PSP
		if (img == osl_curBuf)			{
			//On doit d�sactiver (temporairement) le texturage 2D
			int bTextureEnabled = glIsEnabled(GL_TEXTURE_2D), bBlendingEnabled = glIsEnabled(GL_BLEND);
			glDisable(GL_TEXTURE_2D);
			glDisable(GL_BLEND);
			//Lit les donn�es du backbuffer dans l'image utilis�e jusque l� comme drawbuffer
			glRasterPos2i(0, 0);
			glPixelZoom(1, -1);
			glDrawPixels(img->sysSizeX, img->sizeY, GL_RGBA, emu_pixelPhysFormats[img->pixelFormat], img->data);
			//R�active si jamais
			if (bTextureEnabled)
				glEnable(GL_TEXTURE_2D);
			if (bBlendingEnabled)
				glEnable(GL_BLEND);
		}
	#endif
}

