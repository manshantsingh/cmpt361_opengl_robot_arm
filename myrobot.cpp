
#include "Angel.h"

#include <string>

typedef Angel::vec4 point4;
typedef Angel::vec4 color4;

const int WINDOWS_X = 512;
const int WINDOWS_Y = 512;

const float UPDATE_INTERVAL = 10.0;
const float ANGLE_INCREMENT_VALUE = 1;

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
const GLfloat LOWER_ARM_WIDTH  = 1.0;
const GLfloat UPPER_ARM_HEIGHT = 5.0;
const GLfloat UPPER_ARM_WIDTH  = 1.0;

// Shader transformation matrices
mat4  model_view;

GLuint program, cuboid_vao, cuboid_outline_vao, sphere_fan_vao, sphere_quad_vao;
GLuint vPosition, vColor, ModelView, Projection;

// Array of rotation angles (in degrees) for each rotation axis
enum { Base = 0, LowerArm = 1, UpperArm = 2, NumAngles = 3 };
int      Axis = Base;
GLfloat  Theta[NumAngles] = { 0.0 };

// Menu option values
const int  Quit = 4;
const int ChangeView = -1;

enum AnimationState{ AT_OLD, ATTACHED_TO_ARM, AT_NEW, ALL_DONE} currentAnimationState;
point4 oldPosition, newPosition;
bool isTopView;
GLfloat currentWindowAspect;
vec3 goalRotation;

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

vec3 compute_robot_rotation(point4 p){
    const float RadiansToDegrees = 180.0 / M_PI; // M_PI = 3.14159...

    vec3 result;

    // base direction
    vec2 bi(-1, 0);
    vec2 bf(p.x, p.z);
    float bf_mag = bf.x*bf.x + bf.y*bf.y;
    if(bf_mag != 0.0){
        float angle = acos(dot(bi, bf)/(sqrt(bi.x*bi.x + bi.y*bi.y)*sqrt(bf_mag)));
        float cross = bf.x * bi.y - bf.y * bi.x;
        if(cross<0.0){
            angle = -angle;
        }
        result.x = angle * RadiansToDegrees;
    }
    else result.x = 0;

    float a = LOWER_ARM_HEIGHT;
    float b = UPPER_ARM_HEIGHT + UPPER_ARM_WIDTH/2;

    float diffY = p.y - BASE_HEIGHT;
    float c = sqrt(bf_mag + diffY*diffY);

    float oppC = acos((a*a + b*b - c*c)/(2*a*b)) * RadiansToDegrees;
    float oppB = acos((a*a + c*c - b*b)/(2*a*c)) * RadiansToDegrees;
    float bottomAngle = asin(diffY/c) * RadiansToDegrees;

    result.y = 90 - (bottomAngle + oppB);
    result.z = 180 - oppC;

    // Theta[Base] = result.x;
    // Theta[LowerArm] = result.y;
    // Theta[UpperArm] = result.z;
    // std::cout<<"oopB: "<<oppB<<",\tbottomAngle: "<<bottomAngle<<"\n";

    // printf("bf_mag: %f\ndiffY: %f\na: %f\nb: %f\nc: %f\noppC: %f\noppB: %f\nbottomAngle: %f\n",
    //     bf_mag, diffY, a, b, c, oppC, oppB, bottomAngle);
    // for(auto x: Theta) std::cout<<x<<'\t'; std::cout<<std::endl;
    return result;
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

    glBindVertexArray( cuboid_vao );
    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
    glDrawArrays( GL_TRIANGLES, 0, NumCuboidVertices );

    glBindVertexArray( cuboid_outline_vao );
    glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
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

    glBindVertexArray( cuboid_vao );
    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
    glDrawArrays( GL_TRIANGLES, 0, NumCuboidVertices );

    glBindVertexArray( cuboid_outline_vao );
    glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
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

    glBindVertexArray( cuboid_vao );
    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
    glDrawArrays( GL_TRIANGLES, 0, NumCuboidVertices );

    glBindVertexArray( cuboid_outline_vao );
    glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
    glDrawArrays( GL_TRIANGLES, 0, NumCuboidVertices );
}

//----------------------------------------------------------------------------

void draw_sphere(mat4 m = Translate( 0.0, 0.5 * UPPER_ARM_WIDTH, 0.0 ))
{
    // TODO: msk change the bottom one
    mat4 instance = ( m *
              Scale( UPPER_ARM_WIDTH *0.5,
                 UPPER_ARM_WIDTH *0.5,
                 UPPER_ARM_WIDTH *0.5 ) );

    glUniformMatrix4fv( ModelView, 1, GL_TRUE, model_view * instance );
    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );

    glBindVertexArray( sphere_quad_vao );
    glDrawArrays( GL_TRIANGLE_STRIP, 0, NumSphereQuadVertices );

    glBindVertexArray( sphere_fan_vao );
    glDrawArrays( GL_TRIANGLE_FAN, 0, NumSphereFanVertices );
}

//----------------------------------------------------------------------------

void display( void )
{
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    if(isTopView){
        model_view = RotateX(90);
    }
    else{
        model_view = mat4(1.0);
    }

    if(currentAnimationState == AT_OLD){
        draw_sphere(Translate(oldPosition));
    }
    else if(currentAnimationState == AT_NEW || currentAnimationState == ALL_DONE){
        draw_sphere(Translate(newPosition));
    }

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

void init_cuboid_outline()
{
    // Create a vertex array object
    glGenVertexArrays( 1, &cuboid_outline_vao );
    glBindVertexArray( cuboid_outline_vao );

    color4 colors[NumCuboidVertices];
    for(auto& x: colors) x=vertex_colors[0];

    // Create and initialize a buffer object
    GLuint buffer;
    glGenBuffers( 1, &buffer );
    glBindBuffer( GL_ARRAY_BUFFER, buffer );

    // glBufferData( GL_ARRAY_BUFFER, sizeof(cuboidPoints), cuboidPoints, GL_DYNAMIC_DRAW );

    glBufferData( GL_ARRAY_BUFFER, 
        sizeof(cuboidPoints) + sizeof(colors),
        NULL, GL_DYNAMIC_DRAW );
    glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(cuboidPoints), cuboidPoints );
    glBufferSubData( GL_ARRAY_BUFFER, sizeof(cuboidPoints), sizeof(colors), colors );

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
    init_cuboid_outline();
    compute_sphere();
    init_sphere_quad();
    init_sphere_fan();

    glEnable( GL_DEPTH_TEST );
    glDepthFunc(GL_LESS);

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
    for(auto x: Theta) std::cout<<x<<'\t'; std::cout<<std::endl;
}

//----------------------------------------------------------------------------

void setProjectionMatrix(){

    GLfloat  left = -15.0, right = 15.0;
    GLfloat  bottom = -10.0, top = 20.0;
    GLfloat  zNear = -15.0, zFar = 15.0;

    if ( isTopView ) {
        bottom = -15.0;
        top    =  15.0;
    }

    if ( currentWindowAspect > 1.0 ) {
        left *= currentWindowAspect;
        right *= currentWindowAspect;
    }
    else {
        bottom /= currentWindowAspect;
        top /= currentWindowAspect;
    }

    mat4 projection = Ortho( left, right, bottom, top, zNear, zFar );
    glUniformMatrix4fv( Projection, 1, GL_TRUE, projection );

    model_view = mat4( 1.0 );  // An Identity matrix
}

void menu( int option )
{
    if ( option == Quit ) {
	   exit( EXIT_SUCCESS );
    }
    else if(option == ChangeView){
        isTopView = !isTopView;
        setProjectionMatrix();
        glutPostRedisplay();
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
    currentWindowAspect = GLfloat(width)/height;
    setProjectionMatrix();
}

//----------------------------------------------------------------------------

void keyboard( unsigned char key, int x, int y )
{
    switch( key ) {
	case 033: // Escape Key
	case 'q':
    case 'Q':
	    exit( EXIT_SUCCESS );
	    break;
    case '/':
    case '0':
    case '1':
    case '2':
        menu(key - ((int)'0'));
        break;
    }
}

//----------------------------------------------------------------------------

bool axisReached(int axis, float val){
    // printf("doing axis %d with val %f\t\n", axis, val);
    float old = Theta[axis];
    float diff = val - old;
    if(abs(diff) < ANGLE_INCREMENT_VALUE){
        Theta[axis] = val;
        return true;
    }
    if(diff > 0.0) Theta[axis] += ANGLE_INCREMENT_VALUE;
    else Theta[axis] -= ANGLE_INCREMENT_VALUE;
    return false;
}

void update(int){
    int result = axisReached(Base, goalRotation.x)
                    & axisReached(LowerArm, goalRotation.y)
                    & axisReached(UpperArm, goalRotation.z);

    glutPostRedisplay();

    if (result!=0){
        switch(currentAnimationState){
            case AT_OLD:
                goalRotation = compute_robot_rotation(newPosition);
                currentAnimationState = ATTACHED_TO_ARM;
                break;
            case ATTACHED_TO_ARM:
                goalRotation = vec3(0,0,0);
                currentAnimationState = AT_NEW;
                break;
            case AT_NEW:
                currentAnimationState = ALL_DONE;
                return;
        }
    }
    // printf("gets here\n");
    glutTimerFunc(UPDATE_INTERVAL, update, 0);
}

//----------------------------------------------------------------------------

int main( int argc, char **argv )
{
    if(argc < 8){
        printf("Not enough command line arguments provided.\nAborting.\n");
        return 0;
    }
    // printf("\n\n\nnum args: %d\n\n\n", argc);
    // for(int i=0;i<argc; i++){
    //     printf("args %d: %s\n\n", i, argv[i]);
    // }

    oldPosition = point4(atof(argv[1]), atof(argv[2]), atof(argv[3]), 1);
    newPosition = point4(atof(argv[4]), atof(argv[5]), atof(argv[6]), 1);

    std::string viewSpecified = argv[7];
    if(viewSpecified == "-tv"){
        isTopView = true;
    }
    else if(viewSpecified == "-sv"){
        isTopView = false;
    }
    else{
        printf("Unknown view specified \"%s\".\nAborting.\n", argv[7]);
        return 0;
    }

    currentAnimationState = AT_OLD;
    goalRotation = compute_robot_rotation(oldPosition);

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
    glutAddMenuEntry( "Change View", ChangeView );
    glutAddMenuEntry( "base", Base );
    glutAddMenuEntry( "lower arm", LowerArm );
    glutAddMenuEntry( "upper arm", UpperArm );
    glutAddMenuEntry( "quit", Quit );
    glutAttachMenu( GLUT_MIDDLE_BUTTON );

    glutTimerFunc(UPDATE_INTERVAL, update, 0);

    glutMainLoop();
    return 0;
}
