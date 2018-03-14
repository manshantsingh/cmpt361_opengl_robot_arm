
#include "Angel.h"

#include <string>

typedef Angel::vec4 point4;
typedef Angel::vec4 color4;

const int WINDOWS_X = 512;
const int WINDOWS_Y = 512;


const int NumCuboidVertices = 36; //(6 faces)(2 triangles/face)(3 cuboidVertices/triangle)
point4 cuboidPoints[NumCuboidVertices];
color4 cuboidColors[NumCuboidVertices];
point4 cuboidNormals[NumCuboidVertices];

const int NumSphereQuadVertices = 342; // 8 rows of 18 quads
point4 sphereQuadPoints[NumSphereQuadVertices];
color4 sphereQuadNormals[NumSphereQuadVertices];

const int NumSphereFanVertices = 40;
point4 sphereFanPoints[NumSphereFanVertices];
color4 sphereFanNormals[NumSphereFanVertices];

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
// color4 vertex_colors[8] = {
//     color4( 0.0, 0.0, 0.0, 1.0 ),  // black
//     color4( 1.0, 0.0, 0.0, 1.0 ),  // red
//     color4( 1.0, 1.0, 0.0, 1.0 ),  // yellow
//     color4( 0.0, 1.0, 0.0, 1.0 ),  // green
//     color4( 0.0, 0.0, 1.0, 1.0 ),  // blue
//     color4( 1.0, 0.0, 1.0, 1.0 ),  // magenta
//     color4( 1.0, 1.0, 1.0, 1.0 ),  // white
//     color4( 0.0, 1.0, 1.0, 1.0 )   // cyan
// };
color4 vertex_colors[8] = {
    color4( 0.0, 0.0, 0.0, 0.5 ),  // black
    color4( 0.5, 0.0, 0.0, 0.5 ),  // red
    color4( 0.5, 0.5, 0.0, 0.5 ),  // yellow
    color4( 0.0, 0.5, 0.0, 0.5 ),  // green
    color4( 0.0, 0.0, 0.5, 0.5 ),  // blue
    color4( 0.5, 0.0, 0.5, 0.5 ),  // magenta
    color4( 0.5, 0.5, 0.5, 0.5 ),  // white
    color4( 0.0, 0.5, 0.5, 0.5 )   // cyan
};

// Shader transformation matrices
mat4  model_view;

GLfloat currentWindowAspect;
GLuint program, cuboid_vao, cuboid_outline_vao, sphere_fan_vao, sphere_quad_vao;
GLuint vPosition, vNormal, ModelView, Projection, NormalMatrix, TotalColor;


//----------------------------------------------------------------------------

int Index = 0;

void setShaderMatrixes(mat4 tempMV){
    mat4 tempNM = transpose(inverse(tempMV));
    glUniformMatrix4fv( ModelView, 1, GL_TRUE, tempMV );
    glUniformMatrix4fv( NormalMatrix, 1, GL_TRUE, tempNM );
}

void setProjectionMatrix(){

    // mat4 projection = Ortho( left, right, bottom, top, zNear, zFar );
    mat4 projection = Perspective( 100, WINDOWS_X/WINDOWS_Y, 1, 50);
    projection *= LookAt(vec4(0,5,-15,1), vec4(0,0,1,0), vec4(0,1,0,0));
    glUniformMatrix4fv( Projection, 1, GL_TRUE, projection );

    model_view = mat4( 1.0 );  // An Identity matrix
}

point4 triangle_normal(point4 a, point4 b, point4 c){
    // if(straight) 
    return normalize(point4(cross(b-a, c-a), 1));
    // else return normalize(point4(cross(b-a, a-c), 1));
}

void quad( int a, int b, int c, int d, bool straight = true)
{
    point4 quadNormal = triangle_normal(cuboidVertices[a], cuboidVertices[b], cuboidVertices[c]);

    for(int i=0;i<6;i++){
        cuboidNormals[Index+i] = quadNormal;
    }
    cuboidPoints[Index] = cuboidVertices[a]; Index++;
    cuboidPoints[Index] = cuboidVertices[b]; Index++;
    cuboidPoints[Index] = cuboidVertices[c]; Index++;
    cuboidPoints[Index] = cuboidVertices[b]; Index++;
    cuboidPoints[Index] = cuboidVertices[c]; Index++;
    cuboidPoints[Index] = cuboidVertices[d]; Index++;
}

void colorcube()
{
    Index = 0;
    // quad( 1, 0, 3, 2 , false);
    // quad( 2, 3, 7, 6 );
    // // quad( 2, 3, 7, 6 , mat4(RotateZ(90)));
    // // quad( 2, 3, 7, 6 , mat4(RotateZ(180)));
    // // quad( 2, 3, 7, 6 , mat4(RotateZ(270)));
    // // quad( 2, 3, 7, 6 , mat4(RotateY(90)));
    // // quad( 2, 3, 7, 6 , mat4(RotateY(270)));
    // quad( 3, 0, 4, 7 );
    // quad( 6, 5, 1, 2 );
    // quad( 4, 5, 6, 7 );
    // quad( 5, 4, 0, 1 );
    quad(4,5,7,6); //front
    quad(5,1,6,2); //top
    quad(0,4,3,7); //bottom
    quad(0,1,4,5); //left
    quad(7,6,3,2); //right
    quad(3,2,0,1); //back
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

    for(int i=0;i<NumSphereQuadVertices;i++){
        sphereQuadNormals[i] = normalize(sphereQuadPoints[i]-vec3(0.0, 0.0, 0.0));
    }
    for(int i=0;i<NumSphereFanVertices;i++){
        sphereFanNormals[i] = normalize(sphereFanPoints[i]-vec3(0.0, 0.0, 0.0));
    }
}



//----------------------------------------------------------------------------



void draw_cuboid(mat4 m, color4 selectedColor){

    setShaderMatrixes( model_view * m );
    glUniform4fv( TotalColor, 1, selectedColor );

    glBindVertexArray( cuboid_vao );
    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
    glDrawArrays( GL_TRIANGLES, 0, NumCuboidVertices );
}

//----------------------------------------------------------------------------

void draw_sphere(mat4 m, color4 selectedColor)
{
    // TODO: msk change the bottom one
    mat4 instance = ( m * Scale(3,3,3));

    setShaderMatrixes( model_view * instance );
    glUniform4fv( TotalColor, 1, selectedColor );
    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );

    glBindVertexArray( sphere_quad_vao );
    glDrawArrays( GL_TRIANGLE_STRIP, 0, NumSphereQuadVertices );

    glBindVertexArray( sphere_fan_vao );
    // glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
    glDrawArrays( GL_TRIANGLE_FAN, 0, NumSphereFanVertices/2 );
    glDrawArrays( GL_TRIANGLE_FAN, NumSphereFanVertices/2, NumSphereFanVertices/2 );
}

//----------------------------------------------------------------------------

void display( void )
{
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    model_view = mat4(1.0);

    draw_cuboid( Translate( 0, 0, 0 ) * Scale(90, 1, 50), color4(0, 1, 0, 1));
    draw_sphere( Translate( 0, 3, 0 ) * Scale(1, 1, 1), color4(1, 0, 1, 1));

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
        sizeof(cuboidPoints) + sizeof(cuboidNormals),
        NULL, GL_DYNAMIC_DRAW );
    glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(cuboidPoints), cuboidPoints );
    glBufferSubData( GL_ARRAY_BUFFER, sizeof(cuboidPoints), sizeof(cuboidNormals), cuboidNormals );

    glEnableVertexAttribArray( vPosition );
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0,
			   BUFFER_OFFSET(0) );

    glEnableVertexAttribArray( vNormal );
    glVertexAttribPointer( vNormal, 4, GL_FLOAT, GL_FALSE, 0,
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
        sizeof(cuboidPoints) + sizeof(cuboidNormals),
        NULL, GL_DYNAMIC_DRAW );
    glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(cuboidPoints), cuboidPoints );
    glBufferSubData( GL_ARRAY_BUFFER, sizeof(cuboidPoints), sizeof(cuboidNormals), cuboidNormals );


    glEnableVertexAttribArray( vPosition );
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0,
               BUFFER_OFFSET(0) );

    glEnableVertexAttribArray( vNormal );
    glVertexAttribPointer( vNormal, 4, GL_FLOAT, GL_FALSE, 0,
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
        sizeof(sphereQuadPoints) + sizeof(sphereQuadNormals),
        NULL, GL_DYNAMIC_DRAW );
    glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(sphereQuadPoints), sphereQuadPoints );
    glBufferSubData( GL_ARRAY_BUFFER, sizeof(sphereQuadPoints), sizeof(sphereQuadNormals), sphereQuadNormals );

    glEnableVertexAttribArray( vPosition );
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0,
               BUFFER_OFFSET(0) );

    glEnableVertexAttribArray( vNormal );
    glVertexAttribPointer( vNormal, 4, GL_FLOAT, GL_FALSE, 0,
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
        sizeof(sphereFanPoints) + sizeof(sphereFanNormals),
        NULL, GL_DYNAMIC_DRAW );
    glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(sphereFanPoints), sphereFanPoints );
    glBufferSubData( GL_ARRAY_BUFFER, sizeof(sphereFanPoints), sizeof(sphereFanNormals), sphereFanNormals );

    glEnableVertexAttribArray( vPosition );
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0,
               BUFFER_OFFSET(0) );

    glEnableVertexAttribArray( vNormal );
    glVertexAttribPointer( vNormal, 4, GL_FLOAT, GL_FALSE, 0,
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
    }
}


//----------------------------------------------------------------------------

int main( int argc, char **argv )
{
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
    vNormal = glGetAttribLocation( program, "vNormal" );

    ModelView = glGetUniformLocation( program, "ModelView" );
    Projection = glGetUniformLocation( program, "Projection" );
    NormalMatrix = glGetUniformLocation( program, "NormalMatrix" );
    TotalColor = glGetUniformLocation( program, "TotalColor" );

    init();

    glutDisplayFunc( display );
    glutReshapeFunc( reshape );
    glutKeyboardFunc( keyboard );

    glutMainLoop();
    return 0;
}
