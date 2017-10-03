/*
Trabalho Sistema Solar com texturas e iluminação.
Alunos: Luan Rodrigues Silva e Matheus Francisco Machado

Trabalho baseado no código do moodle.
*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string>

#include <GL/glew.h>
#include <GL/freeglut.h>

#include <CImg.h>

#define sun 0
#define mercury 1
#define venus 2
#define earth 3
#define mars 4
#define jupiter 5
#define saturn 6
#define moon 7

GLint programId = 0;
void init (const char * imgDir, int textureId);

GLint loadShader(GLuint shaderId, const char *path) {
	FILE *file = fopen(path,"r");
	if (file == NULL) {
		fprintf(stderr, "Couldn't open file %s for reading\n", path);
		return GL_FALSE;
	}

	int codeBufferSize = 4096;
	int codeSize = 0;
	char *code = (char*)malloc(codeBufferSize * sizeof(char));
	if (code == NULL) {
		fprintf(stderr, "Failed to request %d bytes of memory\n", codeBufferSize);
		return GL_FALSE;
	}

	int rl = 256;
	int r = 0;
	int i = 0;
	while ((r = fread(&code[i], 1, rl, file)) > 0) {
		i += r;
		if ((i + rl) > (codeBufferSize - 1)) {
			codeBufferSize *= 2;
			char *newCode = (char*)malloc(codeBufferSize * sizeof(char));
			if (newCode == NULL) {
				fprintf(stderr, "Failed to request %d bytes of memory\n", codeBufferSize);
				free(code);
				return GL_FALSE;
			}

			for (int j = 0; j < i; j++) {
				code[j] = newCode[j];
			}
			free(code);
			code = newCode;
		}
	}

	code[i] = '\0';

	//printf("%s\n", code);

	const char *src = code;
	glShaderSource(shaderId, 1, &src, NULL);
	glCompileShader(shaderId);

	GLint ret = GL_FALSE;
	int infoLogLength;
	glGetShaderiv(shaderId, GL_COMPILE_STATUS, &ret);
	glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &infoLogLength);
	if ( infoLogLength > 0 ){
		char msg[4096];
		glGetShaderInfoLog(shaderId, infoLogLength, NULL, msg);
		printf("%s\n", msg);
	}

}


GLuint loadShaders(const char *vsPath, const char *fsPath){

	GLuint vsId = glCreateShader(GL_VERTEX_SHADER);
	GLuint fsId = glCreateShader(GL_FRAGMENT_SHADER);

	loadShader(vsId, vsPath);
	loadShader(fsId, fsPath);

	GLuint progId = glCreateProgram();
	glAttachShader(progId, vsId);
	glAttachShader(progId, fsId);
	glLinkProgram(progId);

	GLint ret = GL_FALSE;
	int infoLogLength;
	glGetProgramiv(progId, GL_LINK_STATUS, &ret);
	glGetProgramiv(progId, GL_INFO_LOG_LENGTH, &infoLogLength);
	if ( infoLogLength > 0 ){
		char msg[4096];
		glGetProgramInfoLog(progId, infoLogLength, NULL, msg);
		printf("%s\n", msg);
	}

	glDetachShader(progId, vsId);
	glDetachShader(progId, fsId);

	glDeleteShader(vsId);
	glDeleteShader(fsId);

	return progId;
}



GLfloat angle, fAspect;

GLfloat rotAngle = 0.0f;
GLfloat rotSpeed = 1.0f;
GLfloat lx=0.0f,lz=-10.0f;
GLfloat x=0.0f, z=10.0f;
GLfloat deltaAngle = 0.0f;
GLfloat deltaMove = 0;
GLint xOrigin = -1;


/*  Create checkerboard texture  */
#define checkImageWidth 16
#define checkImageHeight 16
static GLubyte checkImage[checkImageHeight][checkImageWidth][4];

// suporte até 10 texturas de planeta planetas
static GLuint texName[10];

GLubyte* makeCheckImage(void)
{
   int i, j, c;

   for (i = 0; i < checkImageHeight; i++) {
      for (j = 0; j < checkImageWidth; j++) {
         c = ((((i&0x8)==0)^((j&0x8))==0))*255;
         checkImage[i][j][0] = (GLubyte) c;
         checkImage[i][j][1] = (GLubyte) c;
         checkImage[i][j][2] = (GLubyte) c;
         checkImage[i][j][3] = (GLubyte) 255;
      }
   }

   return (GLubyte*)checkImage;
}

// Inicializa parâmetros de rendering
void init (const char * imgDir, int textureId)
{

	GLfloat luzAmbienteSol[4]={0.2,0.2,0.2,1.0};
	GLfloat luzDifusaSol[4]={0.5f, 0.5f, 0.5f, 1.0f};
	GLfloat luzEspecularSol[4]={1.0, 1.0, 1.0, 1.0};
	GLfloat posicaoLuzSol[4]={10.0, 0.0, 0.0, 1.0};

	GLfloat	luzAmbiente[4]={1.0,1.0,1.0,1.0};
	GLfloat	luzDifusa[4]={1.0f, 1.0f, 1.0f, 1.0f};
	GLfloat	luzEspecular[4]={1.0, 1.0, 1.0, 1.0};
	GLfloat posicaoLuz[4]={10.0, 0.0, 0.0, 1.0};

	GLfloat ambient[4]={1.0,1.0,1.0,1.0};
	GLfloat diffuse[4]={1.0,1.0,1.0,1.0};
	GLfloat specular[4]={1.0,1.0,1.0,1.0};
	
	GLfloat emissive[4]={0.0, 0.0, 0.0, 1.0};

	GLint shininess = 128;

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	glShadeModel(GL_SMOOTH);

	if(textureId == sun) {
		glMaterialfv(GL_FRONT,GL_DIFFUSE, diffuse);
		glMaterialfv(GL_FRONT,GL_AMBIENT, ambient);
		glMaterialfv(GL_FRONT,GL_SPECULAR, specular);
		glMateriali(GL_FRONT,GL_SHININESS, shininess);
		glLightModelfv(GL_LIGHT_MODEL_AMBIENT, luzAmbiente);
		
		glLightfv(GL_LIGHT0, GL_AMBIENT, luzAmbienteSol);
		glLightfv(GL_LIGHT0, GL_DIFFUSE, luzDifusaSol);
		glLightfv(GL_LIGHT0, GL_SPECULAR, luzEspecularSol);
		glLightfv(GL_LIGHT0, GL_POSITION, posicaoLuzSol);
		glEnable(GL_LIGHTING);
		
	} else {
		glMaterialfv(GL_FRONT,GL_DIFFUSE, diffuse);
		glMaterialfv(GL_FRONT,GL_AMBIENT, ambient);
		glMaterialfv(GL_FRONT,GL_SPECULAR, specular);
		glMateriali(GL_FRONT,GL_SHININESS, shininess-80);
		glLightModelfv(GL_LIGHT_MODEL_AMBIENT, luzAmbiente);

		glLightfv(GL_LIGHT0, GL_AMBIENT, luzAmbiente);
		glLightfv(GL_LIGHT0, GL_DIFFUSE, luzDifusa );
		glLightfv(GL_LIGHT0, GL_SPECULAR, luzEspecular );
		glLightfv(GL_LIGHT0, GL_POSITION, posicaoLuz );
	}
	
	
	
	glEnable(GL_LIGHT0);
	glEnable(GL_COLOR_MATERIAL);
	
	glEnable(GL_DEPTH_TEST);

	angle = 45;

	#define LOAD_FROM_FILE
	#ifdef LOAD_FROM_FILE
		// textures
		//cimg_library::CImg<unsigned char> image("img/checker.jpg");
		//cimg_library::CImg<unsigned char> image("img/Lenna.png");
		cimg_library::CImg<unsigned char> image(imgDir);
		GLubyte *imgBuffer = (GLubyte*)malloc(image.width() * image.height() * 4 * sizeof(GLubyte));
		
		cimg_forXY(image,x,y) {
			imgBuffer[(image.width() * y + x) * 4 + 0] = image(x,y,0);
			imgBuffer[(image.width() * y + x) * 4 + 1] = image(x,y,1);
			imgBuffer[(image.width() * y + x) * 4 + 2] = image(x,y,2);
			imgBuffer[(image.width() * y + x) * 4 + 3] = 1.0;
		};
		GLsizei width = image.width();
		GLsizei height = image.height();
	#else
		GLubyte *imgBuffer = makeCheckImage();
		GLsizei width = checkImageWidth;
		GLsizei height = checkImageHeight;
	#endif
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		// gera nome da textura
		glGenTextures(1, &texName[textureId]);
		// 
		glBindTexture(GL_TEXTURE_2D, texName[textureId]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width,
							height, 0, GL_RGBA, GL_UNSIGNED_BYTE,
							imgBuffer);
		glGenerateMipmap(GL_TEXTURE_2D);

	#ifdef LOAD_FROM_FILE
		// descarrega buffer
		free(imgBuffer);
	#endif	
}


void drawTexturedSphere(int texture, GLfloat radius) {

	glRotatef(90, 1.0, 0.0, 0.0);
	glEnable(GL_TEXTURE_2D);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texName[texture]);

	GLUquadric *quad = gluNewQuadric();
	gluQuadricDrawStyle( quad, GLU_FILL);
	gluQuadricNormals( quad, GLU_SMOOTH);
	gluQuadricOrientation( quad, GLU_OUTSIDE);
	gluQuadricTexture( quad, GL_TRUE);
	
	
	gluSphere(quad, radius, 36, 24);

	glDisable(GL_TEXTURE_2D);
}


void draw(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

//	glColor3f(0.0f, 0.0f, 1.0f);

	glUseProgram(programId);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// Set the camera
	//gluLookAt(	x, 1.0f, z,
	//		x+lx, 1.0f,  z+lz,
	//		0.0f, 1.0f,  0.0f);
	gluLookAt(	50.0f, 0.0f, 55.0,
				0.0, 0.0f,  0.0,
				0.0f, 1.0f,  0.0f);
	//gluLookAt(0,0,10, 0,0,0, 0,1,0);
	rotAngle += rotSpeed;

    // sol
	glPushMatrix();
	drawTexturedSphere(sun, 6.0f);
	glPopMatrix();

	

	//mercurio
	glPushMatrix();
	glRotatef(rotAngle*0.5,0.0f,1.0f,0.0f);
	glTranslated(10.0, 0.0, 0.0);
	glRotatef(rotAngle*0.8,0.0f,1.0f,0.0f);
	drawTexturedSphere(mercury, 1.0f);
	glPopMatrix();

	//venus
	glPushMatrix();
	glRotatef(rotAngle*0.8,0.0f,1.0f,0.0f);
	glTranslated(15.0, 0.0, 0.0);
	glRotatef(rotAngle*0.5,0.0f,1.0f,0.0f);
	drawTexturedSphere(venus, 1.5f);
	glPopMatrix();

	//terra
	glPushMatrix();
	glRotatef(rotAngle*0.3,0.0f,1.0f,0.0f);
	glTranslated(20.0, 0.0, 0.0);
	glRotatef(rotAngle*0.8,0.0f,1.0f,0.0f);
	drawTexturedSphere(earth, 3.0f);
	//lua
	glPushMatrix();
		glRotatef(rotAngle*1.5,0.0f,0.2f,1.0f);
		glTranslated(5.0, 0.0, 0.0);
		glRotatef(rotAngle*0.8,0.0f,1.0f,0.0f);
		drawTexturedSphere(moon, 0.7f);
	glPopMatrix();
	glPopMatrix();

	//marte
	glPushMatrix();
	glRotatef(rotAngle*0.4,0.0f,1.0f,0.0f);
	glTranslated(28.0, 0.0, 0.0);
	glRotatef(rotAngle*0.6,0.0f,1.0f,0.0f);
	drawTexturedSphere(mars, 1.3f);
	glUseProgram(0);

	//jupiter
	//glPushMatrix();
	//glRotatef(rotAngle*0.3,0.0f,1.0f,0.0f);
	//glTranslated(40.0, 0.0, 0.0);
	//glRotatef(rotAngle*0.4,0.0f,1.0f,0.0f);
	//drawTexturedSphere(jupiter, 5.0f);

	//saturn
	//glPushMatrix();
	//glRotatef(rotAngle*0.3,0.0f,1.0f,0.0f);
	//glTranslated(40.0, 0.0, 0.0);
	//glRotatef(rotAngle*0.4,0.0f,1.0f,0.0f);
	//drawTexturedSphere(saturn, 5.0f);
	//draw Torus

	//uranus
	//glPushMatrix();
	//glRotatef(rotAngle*0.3,0.0f,1.0f,0.0f);
	//glTranslated(40.0, 0.0, 0.0);
	//glRotatef(rotAngle*0.4,0.0f,1.0f,0.0f);
	//drawTexturedSphere(uranus, 5.0f);

	//neptune
	//glPushMatrix();
	//glRotatef(rotAngle*0.3,0.0f,1.0f,0.0f);
	//glTranslated(40.0, 0.0, 0.0);
	//glRotatef(rotAngle*0.4,0.0f,1.0f,0.0f);
	//drawTexturedSphere(neptune, 5.0f);

	glutSwapBuffers();
}

// Função usada para especificar o volume de visualização
void setup(void)
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	//glOrtho(-5.0,5.0,-5.0,5.0,0.4,500);
	gluPerspective(angle,fAspect,2,200);

	//glEnable(GL_CULL_FACE);
	//glCullFace(GL_FRONT);
}

// Função callback chamada quando o tamanho da janela é alterado
void resize(GLsizei w, GLsizei h)
{
	// Para previnir uma divisão por zero
	if ( h == 0 ) h = 1;

	// Especifica o tamanho da viewport
	glViewport(0, 0, w, h);

	// Calcula a correção de aspecto
	fAspect = (GLfloat)w/(GLfloat)h;

	setup();
}

// Função callback chamada para gerenciar eventos do mouse
void mouse(int button, int state, int x, int y)
{
	// only start motion if the left button is pressed
	if (button == GLUT_LEFT_BUTTON) {

		// when the button is released
		if (state == GLUT_UP) {
			rotAngle += deltaAngle;
			xOrigin = -1;
		}
		else  {// state = GLUT_DOWN
			xOrigin = x;
		}
	}

	setup();
	//glutPostRedisplay();
}

void motion(int x, int y) {

	// this will only be true when the left button is down
	if (xOrigin >= 0) {

		// update deltaAngle
		deltaAngle = (x - xOrigin) * 0.01f;

		// update camera's direction
		lx = sin(rotAngle + deltaAngle) * 10.0;
		lz = -cos(rotAngle + deltaAngle) * 10.0;
	}

	//glutPostRedisplay();
}

void timer(int value)
{
	glutPostRedisplay();
	glutTimerFunc(33,timer, 1);
}

void initShaders() {
	GLenum err = glewInit();
	if (GLEW_OK != err)
	  fprintf(stderr, "%s\n", glewGetErrorString(err));
//	/printf("Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));

	programId = loadShaders("shaders/vertex.glsl","shaders/fragment.glsl");
}

int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

	glutInitWindowSize(400,350);
	glutCreateWindow("Earth");
	glutDisplayFunc(draw);
	glutReshapeFunc(resize);
	glutMouseFunc(mouse);
	glutMotionFunc(motion);
	glutTimerFunc(33,timer,1);

	initShaders();

	init("img/earth.jpg", earth);
	init("img/moon.jpg", moon);
	init("img/sum.jpg", sun);
	init("img/mercury.jpg", mercury);
	init("img/mars.jpg", mars);
	init("img/saturn.jpg", saturn);
	init("img/venus.png", venus);
	
	init("img/jupiter.jpg", jupiter);
	glutMainLoop();
}
