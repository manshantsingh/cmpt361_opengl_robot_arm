
#include "Angel.h"

typedef Angel::vec4 point4;
typedef Angel::vec4 color4;

const int WINDOWS_X = 512;
const int WINDOWS_Y = 512;

const int NumCuboidVertices = 36; //(6 faces)(2 triangles/face)(3 cuboidVertices/triangle)
point4 cuboidPoints[NumCuboidVertices];
color4 cuboidColors[NumCuboidVertices];

const int NumSphereQuadVertices = 342; // 8 rows of 18 quads
point4 sphereQuadPoints[NumSphereQuadVertices];
color4 sphereQuadColors[NumSphereQuadVertices];

const int NumSphereFanVertices = 40;
point4 sphereFanPoints[NumSphereFanVertices];
color4 sphereFanColors[NumSphereFanVertices];

point4 cuboidVertices[8] = {
    point4( -0.5, -0.5,  0.5, 1.0 ),
    point4( -0.5,  0.5,  0.5, 1.0 ),
    point4(  0.5,  0.5,  0.5, 1.0 ),
    point4(  0.5, -0.5,  0.5, 1.0 ),
    point4( -0.5, -0.5, -0.5, 1.0 ),
    point4( -0.5,  0.5, -0.5, 1.0 ),
    point4(  0.5,  0.5, -0.5, 1.0 ),
    point4(  0.5, -0.5, -0.5, 1.0 )
};

// RGBA olors
color4 vertex_colors[8] = {
    color4( 0.0, 0.0, 0.0, 1.0 ),  // black
    color4( 1.0, 0.0, 0.0, 1.0 ),  // red
    color4( 1.0, 1.0, 0.0, 1.0 ),  // yellow
    color4( 0.0, 1.0, 0.0, 1.0 ),  // green
    color4( 0.0, 0.0, 1.0, 1.0 ),  // blue
    color4( 1.0, 0.0, 1.0, 1.0 ),  // magenta
    color4( 1.0, 1.0, 1.0, 1.0 ),  // white
    color4( 0.0, 1.0, 1.0, 1.0 )   // cyan
};


// Parameters controlling the size of the Robot's arm
const GLfloat BASE_HEIGHT      = 2.0;
const GLfloat BASE_WIDTH       = 5.0;
const GLfloat LOWER_ARM_HEIGHT = 5.0;
const GLfloat LOWER_ARM_WIDTH  = 0.5;
const GLfloat UPPER_ARM_HEIGHT = 5.0;
const GLfloat UPPER_ARM_WIDTH  = 0.5;

// Shader transformation matrices
mat4  model_view;
// GLuint ChosenColor;

GLuint program, cuboid_vao, sphere_fan_vao, sphere_quad_vao;
GLuint vPosition, vColor, ModelView, Projection;

// Array of rotation angles (in degrees) for each rotation axis
enum { Base = 0, LowerArm = 1, UpperArm = 2, NumAngles = 3 };
int      Axis = Base;
GLfloat  Theta[NumAngles] = { 0.0 };

// Menu option values
const int  Quit = 4;

enum AnimationState{ AT_OLD, ATTACHED_TO_ARM, AT_NEW, ALL_DONE} currentAnimationState;
point4 oldPosition, newPosition;

//----------------------------------------------------------------------------

int Index = 0;

void quad( int a, int b, int c, int d )
{
    cuboidColors[Index] = vertex_colors[a];
    cuboidPoints[Index] = cuboidVertices[a]; Index++;
    cuboidColors[Index] = vertex_colors[a];
    cuboidPoints[Index] = cuboidVertices[b]; Index++;
    cuboidColors[Index] = vertex_colors[a];
    cuboidPoints[Index] = cuboidVertices[c]; Index++;
    cuboidColors[Index] = vertex_colors[a];
    cuboidPoints[Index] = cuboidVertices[a]; Index++;
    cuboidColors[Index] = vertex_colors[a];
    cuboidPoints[Index] = cuboidVertices[c]; Index++;
    cuboidColors[Index] = vertex_colors[a];
    cuboidPoints[Index] = cuboidVertices[d]; Index++;
}

void colorcube()
{
    quad( 1, 0, 3, 2 );
    quad( 2, 3, 7, 6 );
    quad( 3, 0, 4, 7 );
    quad( 6, 5, 1, 2 );
    quad( 4, 5, 6, 7 );
    quad( 5, 4, 0, 1 );
}

void compute_sphere(){
    const float DegreesToRadians = M_PI / 180.0; // M_PI = 3.14159...
    int k = 0;

    for(float phi = -80.0; phi <= 80.0; phi += 20.0)
    {
        float phir = phi*DegreesToRadians;
        float phir20 = (phi + 20.0)*DegreesToRadians;
        for(float theta = -180.0; theta <= 180.0; theta += 20.0)
        {
            float thetar = theta*DegreesToRadians;
            sphereQuadPoints[k] = vec3(sin(thetar)*cos(phir),
            cos(thetar)*cos(phir), sin(phir));
            k++;
            sphereQuadPoints[k] = vec3(sin(thetar)*cos(phir20),
            cos(thetar)*cos(phir20), sin(phir20));
            k++;
        }
    }

    k = 0;
    sphereFanPoints[k] = vec3(0.0, 0.0, 1.0);
    k++;
    float sin80 = sin(80.0*DegreesToRadians);
    float cos80 = cos(80.0*DegreesToRadians);

    for(float theta = -180.0; theta <= 180.0; theta += 20.0)
    {
        float thetar = theta*DegreesToRadians;
        sphereFanPoints[k] = vec3(sin(thetar)*cos80,
        cos(thetar)*cos80, sin80);
        k++;
    }
    sphereFanPoints[k] = vec3(0.0, 0.0, -1.0);
    k++;
    for(float theta = -180.0; theta <= 180.0; theta += 20.0)
    {
        float thetar = theta*DegreesToRadians;
        sphereFanPoints[k] = vec3(sin(thetar)*cos80,
        cos(thetar)*cos80, -sin80);
        k++;
    }

    for(auto& x: sphereFanColors) {
        x = vertex_colors[1];
    }
    for(auto& x: sphereQuadColors) {
        x = vertex_colors[1];
    }
}

//----------------------------------------------------------------------------

/* Define the three parts */
/* Note use of push/pop to return modelview matrix
to its state before functions were entered and use
rotation, translation, and scaling to create instances
of symbols (cube and cylinder */

void base()
{
    mat4 instance = ( Translate( 0.0, 0.5 * BASE_HEIGHT, 0.0 ) *
		 Scale( BASE_WIDTH,
			BASE_HEIGHT,
			BASE_WIDTH ) );

    glUniformMatrix4fv( ModelView, 1, GL_TRUE, model_view * instance );
    glDrawArrays( GL_TRIANGLES, 0, NumCuboidVertices );
}

//----------------------------------------------------------------------------

void upper_arm()
{
    mat4 instance = ( Translate( 0.0, 0.5 * UPPER_ARM_HEIGHT, 0.0 ) *
		      Scale( UPPER_ARM_WIDTH,
			     UPPER_ARM_HEIGHT,
			     UPPER_ARM_WIDTH ) );

    glUniformMatrix4fv( ModelView, 1, GL_TRUE, model_view * instance );
    glDrawArrays( GL_TRIANGLES, 0, NumCuboidVertices );
}

//----------------------------------------------------------------------------

void lower_arm()
{
    mat4 instance = ( Translate( 0.0, 0.5 * LOWER_ARM_HEIGHT, 0.0 ) *
		      Scale( LOWER_ARM_WIDTH,
			     LOWER_ARM_HEIGHT,
			     LOWER_ARM_WIDTH ) );

    glUniformMatrix4fv( ModelView, 1, GL_TRUE, model_view * instance );
    glDrawArrays( GL_TRIANGLES, 0, NumCuboidVertices );
}

//----------------------------------------------------------------------------

void draw_sphere()
{
    // TODO: msk change the bottom one
    mat4 instance = ( Translate( 0.0, 0.5 * UPPER_ARM_WIDTH, 0.0 ) *
              Scale( UPPER_ARM_WIDTH *0.5,
                 UPPER_ARM_WIDTH *0.5,
                 UPPER_ARM_WIDTH *0.5 ) );

    glUniformMatrix4fv( ModelView, 1, GL_TRUE, model_view * instance );

    glBindVertexArray( sphere_quad_vao );
    glDrawArrays( GL_TRIANGLE_STRIP, 0, NumSphereQuadVertices );

    glBindVertexArray( sphere_fan_vao );
    glDrawArrays( GL_TRIANGLE_FAN, 0, NumSphereFanVertices );
}

//----------------------------------------------------------------------------

void display( void )
{
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    if(currentAnimationState == AT_OLD){
        model_view = Translate(oldPosition);
        draw_sphere();
    }
    else if(currentAnimationState == AT_NEW || currentAnimationState == ALL_DONE){
        model_view = Translate(newPosition);
        draw_sphere();
    }

    model_view = mat4( 1.0 );
    // mat4 camera = LookAt(vec4(0, 0, 7, 1), vec4(0, 0, 0, 1), vec4(0, 1, 0, 1));
    // model_view = Perspective(90, ((float)WINDOWS_X)/WINDOWS_Y, -1, 50) * camera;
    // draw_sphere();
    
    glBindVertexArray( cuboid_vao );

    // Accumulate ModelView Matrix as we traverse the tree
    model_view *= RotateY(Theta[Base] );
    base();

    model_view *= ( Translate(0.0, BASE_HEIGHT, 0.0) *
		    RotateZ(Theta[LowerArm]) );
    lower_arm();

    model_view *= ( Translate(0.0, LOWER_ARM_HEIGHT, 0.0) *
		    RotateZ(Theta[UpperArm]) );
    upper_arm();

    if(currentAnimationState == ATTACHED_TO_ARM){
        // transform
        model_view *= Translate(0.0, UPPER_ARM_HEIGHT, 0.0);
        draw_sphere();
    }

    glutSwapBuffers();
}

//----------------------------------------------------------------------------

void init_cuboid()
{
    colorcube();
    
    // Create a vertex array object
    glGenVertexArrays( 1, &cuboid_vao );
    glBindVertexArray( cuboid_vao );

    // Create and initialize a buffer object
    GLuint buffer;
    glGenBuffers( 1, &buffer );
    glBindBuffer( GL_ARRAY_BUFFER, buffer );

    // glBufferData( GL_ARRAY_BUFFER, sizeof(cuboidPoints), cuboidPoints, GL_DYNAMIC_DRAW );

    glBufferData( GL_ARRAY_BUFFER, 
        sizeof(cuboidPoints) + sizeof(cuboidColors),
        NULL, GL_DYNAMIC_DRAW );
    glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(cuboidPoints), cuboidPoints );
    glBufferSubData( GL_ARRAY_BUFFER, sizeof(cuboidPoints), sizeof(cuboidColors), cuboidColors );

    glEnableVertexAttribArray( vPosition );
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0,
			   BUFFER_OFFSET(0) );

    glEnableVertexAttribArray( vColor );
    glVertexAttribPointer( vColor, 4, GL_FLOAT, GL_FALSE, 0,
			   BUFFER_OFFSET(sizeof(cuboidPoints)) );
}

void init_sphere_quad()
{    
    // Create a vertex array object
    glGenVertexArrays( 1, &sphere_quad_vao );
    glBindVertexArray( sphere_quad_vao );

    // Create and initialize a buffer object
    GLuint buffer;
    glGenBuffers( 1, &buffer );
    glBindBuffer( GL_ARRAY_BUFFER, buffer );

    glBufferData( GL_ARRAY_BUFFER, 
        sizeof(sphereQuadPoints) + sizeof(sphereQuadColors),
        NULL, GL_DYNAMIC_DRAW );
    glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(sphereQuadPoints), sphereQuadPoints );
    glBufferSubData( GL_ARRAY_BUFFER, sizeof(sphereQuadPoints), sizeof(sphereQuadColors), sphereQuadColors );

    glEnableVertexAttribArray( vPosition );
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0,
               BUFFER_OFFSET(0) );

    glEnableVertexAttribArray( vColor );
    glVertexAttribPointer( vColor, 4, GL_FLOAT, GL_FALSE, 0,
               BUFFER_OFFSET(sizeof(sphereQuadPoints)) );
}

void init_sphere_fan()
{
    // Create a vertex array object
    glGenVertexArrays( 1, &sphere_fan_vao );
    glBindVertexArray( sphere_fan_vao );

    // Create and initialize a buffer object
    GLuint buffer;
    glGenBuffers( 1, &buffer );
    glBindBuffer( GL_ARRAY_BUFFER, buffer );

    glBufferData( GL_ARRAY_BUFFER, 
        sizeof(sphereFanPoints) + sizeof(sphereFanColors),
        NULL, GL_DYNAMIC_DRAW );
    glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(sphereFanPoints), sphereFanPoints );
    glBufferSubData( GL_ARRAY_BUFFER, sizeof(sphereFanPoints), sizeof(sphereFanColors), sphereFanColors );

    glEnableVertexAttribArray( vPosition );
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0,
               BUFFER_OFFSET(0) );

    glEnableVertexAttribArray( vColor );
    glVertexAttribPointer( vColor, 4, GL_FLOAT, GL_FALSE, 0,
               BUFFER_OFFSET(sizeof(sphereFanPoints)) );
}

void init(){
    init_cuboid();

    compute_sphere();
    init_sphere_quad();
    init_sphere_fan();

    glEnable( GL_DEPTH_TEST );
    glDepthFunc(GL_LESS);
    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );

    glClearColor( 0.0, 1.0, 1.0, 1.0 ); 
}

//----------------------------------------------------------------------------

void mouse( int button, int state, int x, int y )
{

    if ( button == GLUT_LEFT_BUTTON && state == GLUT_DOWN ) {
    	// Incrase the joint angle
    	Theta[Axis] += 5.0;
    	if ( Theta[Axis] > 360.0 ) { Theta[Axis] -= 360.0; }
    }

    if ( button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN ) {
    	// Decrase the joint angle
    	Theta[Axis] -= 5.0;
    	if ( Theta[Axis] < 0.0 ) { Theta[Axis] += 360.0; }
    }
    glutPostRedisplay();
}

//----------------------------------------------------------------------------

void menu( int option )
{
    if ( option == Quit ) {
	exit( EXIT_SUCCESS );
    }
    else {
        printf("%i\n",option);
	Axis = option;
    }
}

//----------------------------------------------------------------------------

void reshape( int width, int height )
{
    glViewport( 0, 0, width, height );

    GLfloat  left = -10.0, right = 10.0;
    GLfloat  bottom = -5.0, top = 15.0;
    GLfloat  zNear = -10.0, zFar = 10.0;

    GLfloat aspect = GLfloat(width)/height;

    if ( aspect > 1.0 ) {
	left *= aspect;
	right *= aspect;
    }
    else {
	bottom /= aspect;
	top /= aspect;
    }

    mat4 projection = Ortho( left, right, bottom, top, zNear, zFar );
    glUniformMatrix4fv( Projection, 1, GL_TRUE, projection );

    model_view = mat4( 1.0 );  // An Identity matrix
}

//----------------------------------------------------------------------------

void keyboard( unsigned char key, int x, int y )
{
    switch( key ) {
	case 033: // Escape Key
	case 'q': case 'Q':
	    exit( EXIT_SUCCESS );
	    break;
    }
}

//----------------------------------------------------------------------------

int main( int argc, char **argv )
{

    // TODO: msk change below. parse command line arguments here
    currentAnimationState = ATTACHED_TO_ARM;
    oldPosition = point4(1,1,1,1);
    newPosition = point4(2,2,2,1);

    glutInit( &argc, argv );
    glutInitDisplayMode( GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH );
    glutInitWindowSize( WINDOWS_X, WINDOWS_Y );
    glutInitContextVersion( 3, 2 );
    glutInitContextProfile( GLUT_CORE_PROFILE );
    glutCreateWindow( "myrobot" );

    // Iff you get a segmentation error at line 34, please uncomment the line below
    glewExperimental = GL_TRUE; 
    glewInit();

    // Load shaders and use the resulting shader program
    program = InitShader( "vshader81.glsl", "fshader81.glsl" );
    glUseProgram( program );
    vPosition = glGetAttribLocation( program, "vPosition" );
    vColor = glGetAttribLocation( program, "vColor" );
    ModelView = glGetUniformLocation( program, "ModelView" );
    Projection = glGetUniformLocation( program, "Projection" );
    
    init();

    glutDisplayFunc( display );
    glutReshapeFunc( reshape );
    glutKeyboardFunc( keyboard );
    glutMouseFunc( mouse );

    glutCreateMenu( menu );
    // Set the menu values to the relevant rotation axis values (or Quit)
    glutAddMenuEntry( "base", Base );
    glutAddMenuEntry( "lower arm", LowerArm );
    glutAddMenuEntry( "upper arm", UpperArm );
    glutAddMenuEntry( "quit", Quit );
    glutAttachMenu( GLUT_MIDDLE_BUTTON );

    glutMainLoop();
    return 0;
}
