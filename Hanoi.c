#include <stdlib.h>
#include <stdio.h>
#include <GL/glut.h>
// konstante za kreiranje cilindara
#define HEIGHT 0.1f
#define BASE 0.025f
#define SLICES 32
#define INNERSLICES 16
#define LOOPS 1  


#define FPS 100
#define Timer_interval 1000.0/FPS

//konfiguracija diska
struct towerCfg {
	GLfloat gap;//razmak izmedju
	GLfloat towerradius;
	GLfloat towerheight;
};

struct action {
	char fromstack;
	char tostack;
	struct action *next;
};
typedef struct action action;

// struktura- lista gde se cuvaju redosledi akcija , odnosno izvrsavanja
struct actions {
	action *first;
	action *last;
};
typedef struct actions actions;

//struktura koja predstavlja disk 
struct disk {
	char color;
	GLfloat radius;
	struct disk *next;
	struct disk *prev;
};
typedef struct disk disk;
// struktura stek, predstavlja kulu/toranj
struct stack {
	disk *bottom;
	disk *top;
};
typedef struct stack stack;

//pocetni broj diskova i inicijalizacije prema vizuelnom izgledu.
int disks=4;  // pocetni broj diskova
GLfloat rotX, rotY, zoom, offsetY = 1.5, speed=0.015;
GLUquadricObj *quadric; //za koriscenje ugradjenih funkcija
GLfloat pos; // parametar animacije
GLboolean fullscreen; // fullscreen promenljiva
stack tower[3]; // kule u obliku steka
float towerheight[3]; // niz visina kula
struct towerCfg towerCfg; //globalna konfiguracija tornja
actions actqueue; //red akcija koje trebaju biti izvrsene
action *curaction; // trenutna akcija
disk *curdisk; //trenutni disk


// prototipi funkcija, radi lakseg pozivanja
void moveDisk();
void hanoiinit(void);
void reset();
void Display(void);
void hanoi(actions *queue, const int n, const char tower1, const char tower2, const char tower3);
void push(stack *tower, disk *item);
disk *pop(stack *tower);
void drawDisk(GLUquadricObj **quadric, const GLfloat outer, const GLfloat inner);
void drawTower(GLUquadricObj **quadric, const GLfloat radius, const GLfloat height);
void drawAllTowers(GLUquadricObj **quadric, const GLfloat radius, const GLfloat height, const GLfloat gap);



//algoritam za resavanje kula hanoja, osnovna rekurzija, smestanje rezultata svakog poziva u 'actqueue'.
void hanoi(actions *queue, const int n, const char tower1, const char tower2, const char tower3) 
{
	action *curaction;
	if(n > 0) 
	{	hanoi(queue, n-1, tower1, tower3, tower2);
		curaction = (action *)malloc(sizeof(action));
		curaction->next = NULL;
		curaction->fromstack = tower1;
		curaction->tostack = tower3;
		if(queue->first == NULL)
			queue->first = curaction;
		if(queue->last != NULL)
			queue->last->next = curaction;
		queue->last = curaction;
		hanoi(queue, n-1, tower2, tower1, tower3);
	}
}

//push funkcija za stavljanje diska na toranj
void push(stack *tower, disk *item) {
	item->next = NULL;
	if(tower->bottom == NULL) {
		tower->bottom = item;
		tower->top = item;
		item->prev = NULL;
	} else {
		tower->top->next = item;
		item->prev = tower->top;
		tower->top = item;
	}
}
//pop funkcija za skidanje diska sa tornja

disk *pop(stack *tower) {
	disk *tmp;
	if(tower->top != NULL) {
		tmp = tower->top;
		if(tower->top->prev != NULL) {
			tower->top->prev->next = NULL;
			tower->top = tmp->prev;
		} else {
			tower->bottom = NULL;
			tower->top = NULL;
		}
		return tmp;
	}
	return NULL;
}
//Funkcija koja iscrtava disk - Cilindar i dva diska - vrh i osnova.
void drawDisk(GLUquadricObj **quadric, const GLfloat outer, const GLfloat inner)
 {
	glPushMatrix();
		glRotatef(-90.0, 1.0, 0.0, 0.0);
		gluCylinder(*quadric, outer, outer, HEIGHT, SLICES, LOOPS);
		gluQuadricOrientation(*quadric, GLU_INSIDE);
        gluDisk(*quadric, inner, outer, SLICES, LOOPS);
		gluQuadricOrientation(*quadric, GLU_OUTSIDE);
		glTranslatef(0.0, 0.0, HEIGHT);
		gluDisk(*quadric, inner, outer, SLICES, LOOPS);
		gluQuadricOrientation(*quadric, GLU_OUTSIDE);
	glPopMatrix();
}
//Funkcija za crtanje kule
void drawTower(GLUquadricObj **quadric, const GLfloat radius, const GLfloat height) 
{
	glPushMatrix();
		glRotatef(-90.0, 1.0, 0.0, 0.0);
		gluCylinder(*quadric, radius, radius, HEIGHT/2, SLICES, LOOPS);
		gluQuadricOrientation(*quadric, GLU_INSIDE);
		gluDisk(*quadric, 0.0, radius, SLICES, LOOPS);
		gluQuadricOrientation(*quadric, GLU_OUTSIDE);
		glTranslatef(0.0, 0.0, HEIGHT/2);
		gluDisk(*quadric, 0.0, radius, SLICES, LOOPS);
        gluCylinder(*quadric, BASE, BASE, height, INNERSLICES, LOOPS);
		glTranslatef(0.0, 0.0, height);
		gluDisk(*quadric, 0.0, BASE, INNERSLICES, LOOPS);
	glPopMatrix();
}
//Inicijalno crtanje tornjeva
void drawAllTowers(GLUquadricObj **quadric, const GLfloat radius, const GLfloat height, const GLfloat gap) 
{
	glPushMatrix();
		drawTower(quadric, radius, height);
		glTranslatef(-gap, 0.0, 0.0);
		drawTower(quadric, radius, height);
		glTranslatef(gap*2, 0.0, 0.0);
		drawTower(quadric, radius, height);
	glPopMatrix();
}

//Punjenje prve kule

void populatetower(void) 
{
	int i;
	disk *cur;
	GLfloat radius = 0.1f*disks;
	for(i = 0;i < disks;i++) 
	{
		cur = (disk *)malloc(sizeof(disk));
		cur->color = i%2;
		cur->radius = radius;
		push(&tower[0], cur);
		radius -= 0.1;
	}
}
//funkcija za brisanje kula
void cleartowers(void) 
{
	int i;
	disk *cur, *tmp;
	free(curdisk);
	curdisk = NULL;
	for(i = 0;i < 3;i++) 
	{
		cur = tower[i].top;
		while(cur != NULL) 
		{
			tmp = cur->prev;
			free(cur);
			cur = tmp;
		}
		tower[i].top = NULL;
		tower[i].bottom = NULL;
	}
}
// Odavde sve pocinje 
void hanoiinit(void) 
{
	GLfloat radius;
	speed = 0.015;
	radius = 0.1f*disks;
	towerCfg.towerradius = radius+0.1f;
	towerCfg.gap = radius*2+0.5f;
	towerCfg.towerheight = disks*HEIGHT+0.2f;
	populatetower();
	actqueue.first = NULL;
	hanoi(&actqueue, disks, 0, 1, 2);
	curaction = actqueue.first;
	curdisk = pop(&tower[(int)curaction->fromstack]);
	pos = 0.001;
}
//restart scene i animacije
void reset(void) 
{
	cleartowers();
	populatetower();
	curaction = actqueue.first;
	curdisk = pop(&tower[(int)curaction->fromstack]);
	pos = 0.001;

}
//Funkcija za postavljanje boje
void setColor(const int color) 
{
	
	switch(color) {
		case 0:
			glColor3f(0.2, 0.3, 0.7);
			break;
		case 1:
			glColor3f(0, 0.4, 0.4);
			break;
	}
	
}


// Neka osnovna podesavanja, boje , osvetljenje
void Init(void)
{
	const GLfloat mat_specular[] = { 1.0, 0.0, 0.0, 1.0 };
	const GLfloat mat_shininess[] = { 50.0 };
	const GLfloat light_position[] = { 0.0, 1.0, 1.0, 0.0 };
	glShadeModel(GL_SMOOTH);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);  
	glClearColor(0.4, 0.0, 0.0, 1.0);   
	glCullFace(GL_BACK);
	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
	glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);
	glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE); 
	glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_COLOR_MATERIAL);
	glEnable(GL_DEPTH_TEST);
	quadric = gluNewQuadric();
	gluQuadricNormals(quadric, GLU_SMOOTH);
}


void Reshape(int width, int height)
{
	glViewport(0, 0, (GLint) width, (GLint) height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60.0, 1.0, 1.0, 75.0);
	glMatrixMode(GL_MODELVIEW);
}

// Podesavanje broja diskova, zoom 
void Key(unsigned char key, int x, int y)
{
	switch(key) 
	{
		case '1':
			disks=1;
			hanoiinit();
			reset();
			offsetY=0.9;
			zoom=-1.3;
			gluLookAt(0.0, 0.9, 3.6+zoom, 0.0, offsetY, 0.0, 0.0, 1.0, 0.0); 
			reset();
			break;
		case '2':
			disks=2;
			hanoiinit();
			reset();
			offsetY=1.1;
			zoom=-0.8;
			gluLookAt(0.0, 0.9, 3.6+zoom, 0.0, offsetY, 0.0, 0.0, 1.0, 0.0);
			reset();
			break;
		case '3':
			disks=3;
			hanoiinit();
			reset();
			offsetY=1.3;
			zoom=-0.3;
			gluLookAt(0.0, 0.9, 3.6+zoom, 0.0, offsetY, 0.0, 0.0, 1.0, 0.0); 
			break;
		case '4':
			disks=4;
			hanoiinit();
			reset();
			offsetY=1.5;
			zoom=0.8;
			gluLookAt(0.0, 0.9, 3.6+zoom, 0.0, offsetY, 0.0, 0.0, 1.0, 0.0); 
			break;
		case '5':
			disks=5;
			hanoiinit();
			reset();
			offsetY=1.7;
			zoom=1.3;
			gluLookAt(0.0, 0.9, 3.6+zoom, 0.0, offsetY, 0.0, 0.0, 1.0, 0.0); 
			break;
		case '6':
			disks=6;
			hanoiinit();
			reset();
			offsetY=1.9;
			zoom=1.8;
			gluLookAt(0.0, 0.9, 3.6+zoom, 0.0, offsetY, 0.0, 0.0, 1.0, 0.0); 
			break;
		case '7':
			disks=7;
			hanoiinit();
			reset();
			offsetY=2.1;
			zoom=2.3;
			gluLookAt(0.0, 0.9, 3.6+zoom, 0.0, offsetY, 0.0, 0.0, 1.0, 0.0);
			break;
		case '8':
			disks=8;
			hanoiinit();
			reset();
			offsetY=2.3;
			zoom=2.8;
			gluLookAt(0.0, 0.9, 3.6+zoom, 0.0, offsetY, 0.0, 0.0, 1.0, 0.0); 
			break;
		case '9':
			disks=9;
			hanoiinit();
			reset();
			offsetY=2.5;
			zoom=3.3;
			gluLookAt(0.0, 0.9, 3.6+zoom, 0.0, offsetY, 0.0, 0.0, 1.0, 0.0); 
			break;
		case 27:
		case 'q':
			exit(EXIT_SUCCESS);
			break;
		case ' ':
			rotX = 0.0;
			rotY = 0.0;
			zoom = 0.0;
			offsetY = 1.5;
			speed = 0.005;
			break;
		case '+':
			zoom -= 0.1;
			break;
		case '-':
			zoom += 0.1;
			break;
		case 'r':
			reset();
			break;
		case 'f':
			if (fullscreen == 0) 
			{
				glutFullScreen();
				fullscreen = 1;
			} 
			else 
			{
				glutReshapeWindow(800, 600);
				glutPositionWindow(50, 50);
				fullscreen = 0;
			}
			break;
		case 's':
			speed += 0.015;
			break;
		case 'x':
			speed -= 0.015;
			if(speed < 0.0)
				speed = 0.0;
			break;
	}
	glutPostRedisplay();
}
// Funkcija za pomeranje kamere 
void RotateFunc(int key, int x, int y)
{
	switch(key) 
	{
		case GLUT_KEY_UP:
			rotX -= 5;
			break;
		case GLUT_KEY_DOWN:
			rotX += 5;
			break;
		case GLUT_KEY_LEFT:
			rotY -= 5;
			break;
		case GLUT_KEY_RIGHT:
			rotY += 5;
			break;
	
	}
	glutPostRedisplay();
}
//funkcija za zum in/out klikovima misa
void mouse(int button, int state, int x, int y)
{
  
  if (button==GLUT_LEFT_BUTTON && state == GLUT_DOWN)
  {
			zoom += 0.1;
  }
  if (button==GLUT_RIGHT_BUTTON && state == GLUT_DOWN)
		zoom -= 0.1;
  if (button==GLUT_MIDDLE_BUTTON && state == GLUT_DOWN)
		reset();
  
	
  }

void Display(void)
{
	disk *cur;
	int i;
	GLfloat movY;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 
	glLoadIdentity(); 
	gluLookAt(0.0, 0.9, 3.6+zoom, 0.0, offsetY, 0.0, 0.0, 1.0, 0.0);
	glRotatef(rotY, 0.0, 1.0, 0.0);	 
	glRotatef(rotX, 1.0, 0.0, 0.0);	 
	glColor3f(0.6, 0.6, 0.0);
	drawAllTowers(&quadric, towerCfg.towerradius, towerCfg.towerheight, towerCfg.gap); 
	glTranslatef(-towerCfg.gap, HEIGHT/2, 0.0);
	glPushMatrix();
	for(i = 0; i < 3; i++) 
	{ /* Punjenje diskovima */
		glPushMatrix();
		towerheight[i] = 0;
		if((cur = tower[i].bottom) != NULL) 
		{
			do {
				setColor(cur->color);
				drawDisk(&quadric, cur->radius, BASE);
				glTranslatef(0.0, HEIGHT, 0.0);
				towerheight[i] += HEIGHT;
				cur = cur->next;
			} while(cur != NULL);
		}
		glPopMatrix();
		glTranslatef(towerCfg.gap, 0.0, 0.0);
	}
	glPopMatrix();
	if(curaction != NULL && curaction->fromstack != -1 && curdisk != NULL) {
        
        //Deo animacije kada se disk izvlaci sa kule
		if(pos <= 1.0) 
		{ 
			movY = pos*(towerCfg.towerheight-towerheight[(int)curaction->fromstack]); //Povecavanje Y do kraja visine kule
            glTranslatef(towerCfg.gap*curaction->fromstack, towerheight[(int)curaction->fromstack]+movY, 0.0); // translacija do vrha kule
            
		} 
		else 
		{ //Deo animacije kada se disk rotira do zeljene kule (menja y koordinata)
			if(pos < 2.0 && curaction->fromstack != curaction->tostack) 
			{
				if(curaction->fromstack != 1 && curaction->tostack != 1) //Ako se krece sa prve ili trece kule na prvu ili trecu 
				{ 
					glTranslatef(towerCfg.gap, towerCfg.towerheight+0.05f, 0.0);
					if(curaction->fromstack == 0)  //Pomeranje sa vrha prve kule na vrh trece kule
						glRotatef(-(pos-2.0f)*180-90, 0.0, 0.0, 1.0);
					else
						glRotatef((pos-2.0f)*180+90, 0.0, 0.0, 1.0); // Pomeranje sa vrha trece kule na vrh prve kule
					    glTranslatef(0.0, towerCfg.gap, 0.0);
				} 
				else 
				{ 
                        if(curaction->fromstack == 0&&curaction->tostack == 1) //Pomeranje sa vrha prve kule na vrh druge kule
                         {
                            glTranslatef(towerCfg.gap/2, towerCfg.towerheight+0.05f, 0.0);
                            glRotatef(-(pos-2.0f)*180-90, 0.0, 0.0, 1.0);
                         } 
                        else 
                        {
                            if(curaction->fromstack == 2 && curaction-> tostack == 1) //Pomeranje sa vrha trece kule na vrh druge kule 
                                {
                                glTranslatef(towerCfg.gap/2*3, towerCfg.towerheight+0.05f, 0.0);
                                glRotatef((pos-2.0f)*180+90, 0.0, 0.0, 1.0);
                                } 
                            else 
                                {
                                    if(curaction->fromstack == 1 && curaction->tostack == 2) //Pomeranje sa vrha druge kule na vrh trece kule
                                        {    
                                        glTranslatef(towerCfg.gap/2*3, towerCfg.towerheight+0.05f, 0.0);
                                        glRotatef(-(pos-2.0f)*180-90, 0.0, 0.0, 1.0);
                                        } 
                                    else //Pomeranje sa vrha druge kule na vrh prve kule
                                        {
                                        glTranslatef(towerCfg.gap/2, towerCfg.towerheight+0.05f, 0.0);
                                        glRotatef((pos-2.0f)*180+90, 0.0, 0.0, 1.0);
                                        }
                                }
                        }
					glTranslatef(0.0, towerCfg.gap/2, 0.0);
				}
				glRotatef(-90, 0.0, 0.0, 1.0);
			} 
			else // deo animacije kada se disk spusta niz kulu
				if(pos >= 2.0) 
				{ 
                movY = towerCfg.towerheight-(pos-2.0f+speed)* (towerCfg.towerheight-towerheight[(int)curaction->tostack]);
                glTranslatef(towerCfg.gap*curaction->tostack, movY, 0.0);
				}
		}
		setColor(curdisk->color);
		drawDisk(&quadric, curdisk->radius, BASE);
	}
	glutSwapBuffers(); 
}
//Funkcija zaduzena za pomeranje diska sa kule na kulu
void moveDisk() 
{
	
	if(curaction != NULL) 
	{
		if(pos == 0.0 || pos >= 3.0-speed) 
		{ 
			pos = 0.0;
			push(&tower[(int)curaction->tostack], curdisk);
			curaction = curaction->next;
			if(curaction != NULL)
				curdisk = pop(&tower[(int)curaction->fromstack]);
		}
		pos += speed;
		if(pos > 3.0)
			pos = 3.0;
      glutTimerFunc((unsigned)Timer_interval, moveDisk, 0);
	} 
	else 
	{
		curdisk = NULL;
		glutTimerFunc(5000, moveDisk, 1);
	}
	glutPostRedisplay();
}


int main(int argc, char *argv[])
{
	hanoiinit();
	glutInit(&argc, argv);
    
	glutInitWindowPosition(0, 0);
	glutInitWindowSize(800, 600);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE);
	if(glutCreateWindow("Kule Hanoja") == GL_FALSE)
		exit(EXIT_FAILURE);
	Init();
	glutReshapeFunc(Reshape);
	glutKeyboardFunc(Key);
	glutSpecialFunc(RotateFunc);
	glutMouseFunc(mouse);
	glutDisplayFunc(Display);
	glutTimerFunc((unsigned)Timer_interval, moveDisk, 0);
	
	glutMainLoop();
	return EXIT_SUCCESS;
}


