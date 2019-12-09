/* U kodu su korisceni delovi koda iskljucivo sa vezbi */
/* Animacije su tek na pocetku i u izradi */


#include <stdlib.h>
#include <GL/glut.h>
#include <math.h>
#include <stdio.h>
#define PI 3.1415926535
#define EPSILON 0.01
#define TIMER_ID 0
#define TIMER_INTERVAL 20


float x1=-2.6,y,x2=-2.6,y2,x3=-2.6,y3;
float x_curr=-2.6, y_curr;    
static float v_x, v_y;          
                                
static int animation_ongoing;

static int timer_active;


static void on_keyboard(unsigned char key, int x, int y);
static void on_reshape(int width, int height);
static void on_display(void);

static void on_timer(int value);


int main(int argc, char **argv)
{
    
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);


    glutInitWindowSize(1200, 800);
    glutInitWindowPosition(100, 100);
    glutCreateWindow(argv[0]);

    
    glutKeyboardFunc(on_keyboard);
    glutReshapeFunc(on_reshape);
    glutDisplayFunc(on_display);

    
    glClearColor(0.4, 0, 0, 0);
    glEnable(GL_DEPTH_TEST);

    
    glEnable(GL_NORMALIZE);

    
    glutMainLoop();

    return 0;
}



static void on_reshape(int width, int height)
{
    
    glViewport(0, 0, width, height);

    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60, (float) width / height, 1, 1500);
}





void set_normal_and_vertex(float u, float v)
{

    glNormal3f(
            sin(v)/5,
            0,
            cos(v)/5
            );
    glVertex3f(
            sin(v)/5,
            u,
            cos(v)/5
            );
}




void draw_torus(float x,float y,float z,float k){
   glPushMatrix();
       glTranslatef(x,y,z);
       glRotatef(90,1,0,0);
   

       glutSolidTorus(k-0.2,k,20,20);

   glPopMatrix();



}
void draw_object(float x , float y, float z)
{
    float u, v;

    glPushMatrix();
    glTranslatef(x,y,z);

    for (u = 1; u < 4; u += PI / 20) {
        glBegin(GL_TRIANGLE_STRIP);
        for (v = 0; v <= PI*2 + EPSILON; v += PI / 20) {
            set_normal_and_vertex(u, v);
            set_normal_and_vertex(u + PI / 20, v);
        }
        glEnd();
    }

    glPopMatrix();
}
static void on_keyboard(unsigned char key, int x, int y)
{
    switch (key) {
    case 27:       
        exit(0);
        break;

    case 'g':
    case 'G':
        if (!animation_ongoing) {
            glutTimerFunc(TIMER_INTERVAL, on_timer, TIMER_ID);
            animation_ongoing = 1;
        }
        break;

    case 's':
    case 'S':
        animation_ongoing = 0;
        break;
    }
}

static void on_timer(int value)
{
    if (value != TIMER_ID)
        return;


    /* Korak 1 */
    if(y_curr<=2 && x_curr<0)
        y_curr+=0.06;
      
    else if(y_curr>=2 && x_curr<0)
	x_curr+=0.06;
    if(x_curr>0)
	x_curr=0;
    if(x_curr==0 && y_curr>=-1.5)
	y_curr-=0.06;

    /* korak 2 */
    if(x_curr==0){
	if(y<=2.5 && x1<2.5)
	y+=0.06;
	else if(y>=2.5 && x1<2.6)
	x1+=0.06;
	if(x1>=2.6)
	x1=2.6;
	if(x1==2.6 && y>=2)
	y-=0.06;
}

    
    glutPostRedisplay();

    if (animation_ongoing) {
        glutTimerFunc(TIMER_INTERVAL, on_timer, TIMER_ID);
    }
}

static void on_display(void)
{
    
    GLfloat light_position[] = { 0, 1, 1, 0 };
 
    GLfloat light_ambient[] = { 1, 0.1, 0, 1 };

    GLfloat light_diffuse[] = { 0.7, 0.7, 0.7, 1 };

    GLfloat light_specular[] = { 0.5, 0, 0, 1 };

    GLfloat ambient_coeffs[] = { 0.2, 0.1, 0.1, 1 };

    GLfloat diffuse_coeffs[] = { 1, 1, 0, 1 };

    GLfloat specular_coeffs[] = { 1, 1, 1, 1 };

    GLfloat shininess = 30;


    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(0, 3, 5, 0, 0, 0, 0, 1, 0);

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);


    glMaterialfv(GL_FRONT, GL_AMBIENT, ambient_coeffs);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse_coeffs);
    glMaterialfv(GL_FRONT, GL_SPECULAR, specular_coeffs);
    glMaterialf(GL_FRONT, GL_SHININESS, shininess);

    glShadeModel(GL_SMOOTH);


    	draw_object(2.6,-3,0);
	draw_object(-2.6,-3,0);
	draw_object(0,-3,0);


	
	draw_torus(-2.6,-1.6,0,0.49);
	draw_torus(-2.6,-1.2,0,0.46);
        draw_torus(x1,y-0.8,0,0.43); /* korak 2 gore */
        draw_torus(x_curr,y_curr-0.4,0,0.40); /* korak 1 gore */

    glutSwapBuffers();
}
