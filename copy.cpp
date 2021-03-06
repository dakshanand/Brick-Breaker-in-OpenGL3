#include <iostream>
#include <cmath>
#include <fstream>
#include <vector>
#include <stdio.h>      /* printf, scanf, puts, NULL */
#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */
#include <math.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;

struct VAO {
  GLuint VertexArrayID;
  GLuint VertexBuffer;
  GLuint ColorBuffer;

  GLenum PrimitiveMode;
  GLenum FillMode;
  int NumVertices;
};
typedef struct VAO VAO;

struct GLMatrices {
  glm::mat4 projection;
  glm::mat4 model;
  glm::mat4 view;
  GLuint MatrixID;
} Matrices;

GLuint programID;

/* Function to load Shaders - Use it as it is */
GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path) {

  // Create the shaders
  GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
  GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

  // Read the Vertex Shader code from the file
  std::string VertexShaderCode;
  std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
  if(VertexShaderStream.is_open())
    {
      std::string Line = "";
      while(getline(VertexShaderStream, Line))
	VertexShaderCode += "\n" + Line;
      VertexShaderStream.close();
    }

  // Read the Fragment Shader code from the file
  std::string FragmentShaderCode;
  std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
  if(FragmentShaderStream.is_open()){
    std::string Line = "";
    while(getline(FragmentShaderStream, Line))
      FragmentShaderCode += "\n" + Line;
    FragmentShaderStream.close();
  }

  GLint Result = GL_FALSE;
  int InfoLogLength;

  // Compile Vertex Shader
  printf("Compiling shader : %s\n", vertex_file_path);
  char const * VertexSourcePointer = VertexShaderCode.c_str();
  glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
  glCompileShader(VertexShaderID);

  // Check Vertex Shader
  glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
  glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
  std::vector<char> VertexShaderErrorMessage(InfoLogLength);
  glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
  fprintf(stdout, "%s\n", &VertexShaderErrorMessage[0]);

  // Compile Fragment Shader
  printf("Compiling shader : %s\n", fragment_file_path);
  char const * FragmentSourcePointer = FragmentShaderCode.c_str();
  glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
  glCompileShader(FragmentShaderID);

  // Check Fragment Shader
  glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
  glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
  std::vector<char> FragmentShaderErrorMessage(InfoLogLength);
  glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
  fprintf(stdout, "%s\n", &FragmentShaderErrorMessage[0]);

  // Link the program
  fprintf(stdout, "Linking program\n");
  GLuint ProgramID = glCreateProgram();
  glAttachShader(ProgramID, VertexShaderID);
  glAttachShader(ProgramID, FragmentShaderID);
  glLinkProgram(ProgramID);

  // Check the program
  glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
  glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
  std::vector<char> ProgramErrorMessage( max(InfoLogLength, int(1)) );
  glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
  fprintf(stdout, "%s\n", &ProgramErrorMessage[0]);

  glDeleteShader(VertexShaderID);
  glDeleteShader(FragmentShaderID);

  return ProgramID;
}

static void error_callback(int error, const char* description)
{
  fprintf(stderr, "Error: %s\n", description);
}

void quit(GLFWwindow *window)
{
  glfwDestroyWindow(window);
  glfwTerminate();
  //    exit(EXIT_SUCCESS);
}

float gen_point()
{
  int r=rand()%700-350;
  float x=r/100.0;
  return x;
}


/* Generate VAO, VBOs and return VAO handle */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat* color_buffer_data, GLenum fill_mode=GL_FILL)
{
  struct VAO* vao = new struct VAO;
  vao->PrimitiveMode = primitive_mode;
  vao->NumVertices = numVertices;
  vao->FillMode = fill_mode;

  // Create Vertex Array Object
  // Should be done after CreateWindow and before any other GL calls
  glGenVertexArrays(1, &(vao->VertexArrayID)); // VAO
  glGenBuffers (1, &(vao->VertexBuffer)); // VBO - vertices
  glGenBuffers (1, &(vao->ColorBuffer));  // VBO - colors

  glBindVertexArray (vao->VertexArrayID); // Bind the VAO 
  glBindBuffer (GL_ARRAY_BUFFER, vao->VertexBuffer); // Bind the VBO vertices 
  glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), vertex_buffer_data, GL_STATIC_DRAW); // Copy the vertices into VBO
  glVertexAttribPointer(
			0,                  // attribute 0. Vertices
			3,                  // size (x,y,z)
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
			);

  glBindBuffer (GL_ARRAY_BUFFER, vao->ColorBuffer); // Bind the VBO colors 
  glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), color_buffer_data, GL_STATIC_DRAW);  // Copy the vertex colors
  glVertexAttribPointer(
			1,                  // attribute 1. Color
			3,                  // size (r,g,b)
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
			);

  return vao;
}

/* Generate VAO, VBOs and return VAO handle - Common Color for all vertices */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat red, const GLfloat green, const GLfloat blue, GLenum fill_mode=GL_FILL)
{
  GLfloat* color_buffer_data = new GLfloat [3*numVertices];
  for (int i=0; i<numVertices; i++) {
    color_buffer_data [3*i] = red;
    color_buffer_data [3*i + 1] = green;
    color_buffer_data [3*i + 2] = blue;
  }

  return create3DObject(primitive_mode, numVertices, vertex_buffer_data, color_buffer_data, fill_mode);
}

/* Render the VBOs handled by VAO */
void draw3DObject (struct VAO* vao)
{
  // Change the Fill Mode for this object
  glPolygonMode (GL_FRONT_AND_BACK, vao->FillMode);

  // Bind the VAO to use
  glBindVertexArray (vao->VertexArrayID);

  // Enable Vertex Attribute 0 - 3d Vertices
  glEnableVertexAttribArray(0);
  // Bind the VBO to use
  glBindBuffer(GL_ARRAY_BUFFER, vao->VertexBuffer);

  // Enable Vertex Attribute 1 - Color
  glEnableVertexAttribArray(1);
  // Bind the VBO to use
  glBindBuffer(GL_ARRAY_BUFFER, vao->ColorBuffer);

  // Draw the geometry !
  glDrawArrays(vao->PrimitiveMode, 0, vao->NumVertices); // Starting from vertex 0; 3 vertices total -> 1 triangle
}

/**************************
 * Customizable functions *
 **************************/

float cannonPos=0,cannonAngle=0;

/* Executed when a regular key is pressed/released/held-down */
/* Prefered for Keyboard events */


/* Executed when a mouse button is pressed/released */
void mouseButton (GLFWwindow* window, int button, int action, int mods)
{
  switch (button) {
  case GLFW_MOUSE_BUTTON_LEFT:
    if (action == GLFW_RELEASE)

      break;
  case GLFW_MOUSE_BUTTON_RIGHT:
    if (action == GLFW_RELEASE) {

    }
    break;
  default:
    break;
  }
}


/* Executed when window is resized to 'width' and 'height' */
/* Modify the bounds of the screen here in glm::ortho or Field of View in glm::Perspective */

void reshapeWindow (GLFWwindow* window, int width, int height)
{
  int fbwidth=width, fbheight=height;
  /* With Retina display on Mac OS X, GLFW's FramebufferSize
     is different from WindowSize */
  glfwGetFramebufferSize(window, &fbwidth, &fbheight);

  GLfloat fov = 90.0f;

  // sets the viewport of openGL renderer
  glViewport (0, 0, (GLsizei) fbwidth, (GLsizei) fbheight);

  // set the projection matrix as perspective
  /* glMatrixMode (GL_PROJECTION);
     glLoadIdentity ();
     gluPerspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1, 500.0); */
  // Store the projection matrix in a variable for future use
  // Perspective projection for 3D views
  // Matrices.projection = glm::perspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1f, 500.0f);

  // Ortho projection for 2D views
  Matrices.projection = glm::ortho(-4.0f, 4.0f, -4.0f, 4.0f, 0.1f, 500.0f);
}
float ab(float x)
{
  float r=x;
  if(r<0)r*=-1;
  return r;
}
VAO *brick,*laser,*mirror,*cannon,*blue,*green;

GLfloat bricks [100000]={0},colbricks[100000]={0},Laser[20]={0},colLaser[20]={0},Mirror[100],colMirror[100]={0},bluePos=0,greenPos=0;
int numBricks=0,numMirrors,score=0,Over=0;
double fall=0,speed=0.0001,angles[10];
void createMirrors()
{
  int i,N,angle;
  float A;
  numMirrors=3;
  for(i=0;i<numMirrors;i++)
    {
 
      angle=rand()%30+60;
      


      angles[i]=angle;
      A=angle;
      A=tan(A/180*M_PI);
      N=i*6;
      Mirror[N]=Mirror[N+3]=gen_point();
      Mirror[N+1]=Mirror[N+4]=gen_point();
      
      while(ab(Mirror[N]-Mirror[N+3])<=1&&ab(Mirror[N+1]-Mirror[N+4])<=1)
	{
	  Mirror[N+3]+=0.01;
	  Mirror[N+4]+=A*0.01;

	}



    }
  mirror= create3DObject(GL_LINES,6*numMirrors,Mirror, colMirror, GL_FILL);

  }
void createLaser()
{
  Laser[0]=-4,Laser[1]=cannonPos;
  float i=-4,j=cannonPos,M,C,angle,dif;
  int flag=1,n,f,k,dir=1,N,LaserN=0;
  angle=cannonAngle;
  while(i<=4&&j<=4&&j>=-4&&i>=-4)
    {

      i+=speed*10*dir;
      j+=(speed*10*tan(angle/180*M_PI)*dir);
      for(k=0;k<numMirrors;k++)
	{
	  M=tan(angles[k]/180*M_PI);
	  N=k*6;
	  C=Mirror[N+1]-M*Mirror[N];
	  if(ab(j-M*i-C)<=0.01&&i>=Mirror[N]&&i<=Mirror[N+3]&&dir==1)
	    {
	      dir*=-1;
	      Laser[LaserN*6+3]=i,Laser[LaserN*6+4]=j;
	      LaserN++;
	      Laser[LaserN*6]=i;
	      Laser[LaserN*6+1]=j;
	      dif=90-ab(angles[k]);
	      /*angle+=dif;
	      angle*=-1;
	      angle-=dif;*/
	      angle=2*angles[k]-angle;
	      break;
	    }
	  
	  
	    }
      for(f=0;f<numBricks;f++)
	{
	  n=18*f;
	  if(i>=bricks[n]&&i<=bricks[n]+0.1&&j<=bricks[n+1]-fall&&j>=bricks[n+1]-0.2-fall)
	    {
	      for( k=0;k<18;k++)
		bricks[n+k]=0;
	      score+=speed/0.0001;
	      
	      brick = create3DObject(GL_TRIANGLES,numBricks*6, bricks, colbricks, GL_FILL);

	      flag=0;break;
	    }
	}
      if(!flag)break;
    }
  Laser[(LaserN*6)+3]=i,Laser[(LaserN*6)+4]=j;
  for(int k=0;k<4;k++)
    colLaser[k*3]=1;
  laser = create3DObject(GL_LINES,20, Laser, colLaser, GL_FILL);
  
}

void createBrick(float x)
{
  // GL3 accepts only Triangles. Quads are not supported

 
  GLfloat vertex[] = {
    x,4+fall,0, // vertex 1
    x+0.1,4+fall,0, // vertex 2
    x+0.1, 3.9+fall,0, // vertex 3

    x+0.1, 3.9+fall,0, // vertex 3
    x, 3.9+fall,0, // vertex 4
    x,4+fall,0  // vertex 1
  };
  int s=numBricks*18;
  int i;
  for(i=0;i<18;i++)
    bricks[s+i]=vertex[i];
  
 
  numBricks++;
  int r=rand()%3;
  if(r)
    {
      for (i=0;i<6;i++)
	colbricks[s+i*3+r]=1;
    }
 

  // create3DObject creates and returns a handle to a VAO that can be used later
  //rectangle = create3DObject(GL_LINES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
  brick = create3DObject(GL_TRIANGLES,numBricks*6, bricks, colbricks, GL_FILL);
}

void checkBlue()
{
  int i,N;
  for(i=0;i<numBricks;i++)
    {
      N=i*18;
      if(!colbricks[N+1]&&!colbricks[N+2])
	if(bricks[N+1]-0.1-fall<=-3.6&&bricks[N]>=2.4+greenPos&&bricks[N]<=3+bluePos);//Over=1;
      if(!colbricks[N+2])
	continue;
      if(bricks[N+1]-0.1-fall<=-3.6&&bricks[N]>=-3+bluePos&&bricks[N]<=-2.4+bluePos)
	{
	   for( int k=0;k<18;k++)
		bricks[N+k]=0;
	       score+=speed/0.0001;
	      brick = create3DObject(GL_TRIANGLES,numBricks*6, bricks, colbricks, GL_FILL);
	}
    }
}


void checkGreen()
{
  int i,N;
  for(i=0;i<numBricks;i++)
    {
      N=i*18;
      if(!colbricks[N+1]&&!colbricks[N+2])
	if(bricks[N+1]-0.1-fall<=-3.6&&bricks[N]>=2.4+greenPos&&bricks[N]<=3+greenPos);//Over=1;
      if(!colbricks[N+1])
	continue;
      if(bricks[N+1]-0.1-fall<=-3.6&&bricks[N]>=2.4+greenPos&&bricks[N]<=3+greenPos)
	{
	   for( int k=0;k<18;k++)
		bricks[N+k]=0;
	    score+=speed/0.0001;
	      
	      brick = create3DObject(GL_TRIANGLES,numBricks*6, bricks, colbricks, GL_FILL);
	}
    }
}

void createCannon()
{
 GLfloat vertex[] = {
    -4,0,0, // vertex 1
    -3.7,0,0, // vertex 2
    -3.7, -0.1,0, // vertex 3

   -3.7, -0.1,0, // vertex 3
    -4, -0.1,0, // vertex 4
    -4,0,0  // vertex 1
  };

  cannon = create3DObject(GL_TRIANGLES,6, vertex, colLaser, GL_LINES);
  
}

void createBuckets()
{
  
 GLfloat vertex[] = {
    -3,-3.6,0, // vertex 1
    -2.4,-3.6,0, // vertex 2
    -2.6, -3.9,0, // vertex 3

    -2.6, -3.9,0, // vertex 3
    -2.8, -3.9,0, // vertex 4
     -3,-3.6,0,  // vertex 1
  };

 GLfloat col[18]={0};
 int i;
 for(i=0;i<6;i++)
   col[i*3+2]=1;
  blue = create3DObject(GL_TRIANGLES,6, vertex, col, GL_LINES);

   GLfloat vertex2[] = {
    3,-3.6,0, // vertex 1
    2.4,-3.6,0, // vertex 2
    2.6, -3.9,0, // vertex 3

    2.6, -3.9,0, // vertex 3
    2.8, -3.9,0, // vertex 4
     3,-3.6,0,  // vertex 1
  };

   GLfloat col2[18]={0};

 for(i=0;i<6;i++)
   col2[i*3+1]=1;
  green = create3DObject(GL_TRIANGLES,6, vertex2, col2, GL_LINES);
  
}
double last_update_time1;

void keyboard (GLFWwindow* window, int key, int scancode, int action, int mods)
{
  // Function is called first on GLFW_PRESS.

  if (action == GLFW_RELEASE) {
    switch (key) {
    case GLFW_KEY_C:
              
      break;
    case GLFW_KEY_P:

      break;
   
    case GLFW_KEY_RIGHT:
    {
      if((glfwGetKey(window,GLFW_KEY_RIGHT_ALT)||glfwGetKey(window,GLFW_KEY_LEFT_ALT))&&bluePos<=6.1)
	bluePos+=0.1;

       if((glfwGetKey(window,GLFW_KEY_RIGHT_CONTROL)||glfwGetKey(window,GLFW_KEY_LEFT_CONTROL))&&greenPos<1)
	greenPos+=0.1;

      break;
    }

     case GLFW_KEY_LEFT:
    {
      if((glfwGetKey(window,GLFW_KEY_RIGHT_ALT)||glfwGetKey(window,GLFW_KEY_LEFT_ALT))&&bluePos>=-1)
	bluePos-=0.1;

       if((glfwGetKey(window,GLFW_KEY_RIGHT_CONTROL)||glfwGetKey(window,GLFW_KEY_LEFT_CONTROL))&&greenPos>=-6.1)
	greenPos-=0.1;
      



      break;
    }
    default:
      break;
    }
  }
  else if (action == GLFW_PRESS) {
    switch (key) {
    case GLFW_KEY_ESCAPE:
      quit(window);
      break;
    default:
      break;
    }
  }
}

/* Executed for character input (like in text boxes) */
void keyboardChar (GLFWwindow* window, unsigned int key)
{
  switch (key) {
  case 'Q':
  case 'q':
    quit(window);
    break;
	    
  case 's':
  case 'S':
    if(cannonPos<3.9)
    cannonPos+=0.05;
    break;

  case 'f':
  case 'F':
    if(cannonPos>-3.9)
    cannonPos-=0.05;
    break;

  case 'a':
  case 'A':
    if(cannonAngle<89)
    cannonAngle+=0.5;
    break;
		  
  case 'd':
  case 'D':
    if(cannonAngle>-89)
    cannonAngle-=0.5;
    break;


  case 'x':
  case 'X':
    {
      if(glfwGetTime()-last_update_time1>=1)
	{
	  createLaser();
	  last_update_time1 = glfwGetTime();
	}
      
      break;
    }
       case 'n':
  case 'N':
    {
      float s=speed/0.0001;
      if(s>1)s--;
      speed=s*0.0001;
      
      break;
    }

    
       case 'm':
  case 'M':
    {
      float s=speed/0.0001;
      if(s<5)s++;
      speed=s*0.0001;
      
      break;
    }



    
  default:
    break;
  }
}

float camera_rotation_angle = 90;

/* Render the scene with openGL */
/* Edit this function according to your assignment */
void draw ()
{
  // clear the color and depth in the frame buffer
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // use the loaded shader program
  // Don't change unless you know what you are doing
  glUseProgram (programID);

  // Eye - Location of camera. Don't change unless you are sure!!
  glm::vec3 eye ( 5*cos(camera_rotation_angle*M_PI/180.0f), 0, 5*sin(camera_rotation_angle*M_PI/180.0f) );
  // Target - Where is the camera looking at.  Don't change unless you are sure!!
  glm::vec3 target (0, 0, 0);
  // Up - Up vector defines tilt of camera.  Don't change unless you are sure!!
  glm::vec3 up (0, 1, 0);

  // Compute Camera matrix (view)
  // Matrices.view = glm::lookAt( eye, target, up ); // Rotating Camera for 3D
  //  Don't change unless you are sure!!
  Matrices.view = glm::lookAt(glm::vec3(0,0,3), glm::vec3(0,0,0), glm::vec3(0,1,0)); // Fixed camera for 2D (ortho) in XY plane

  // Compute ViewProject matrix as view/camera might not be changed for this frame (basic scenario)
  //  Don't change unless you are sure!!
  glm::mat4 VP = Matrices.projection * Matrices.view;

  // Send our transformation to the currently bound shader, in the "MVP" uniform
  // For each model you render, since the MVP will be different (at least the M part)
  //  Don't change unless you are sure!!
  glm::mat4 MVP;	// MVP = Projection * View * Model


 Matrices.model = glm::mat4(1.0f);
 

 glm::mat4 translateB = glm::translate (glm::vec3(bluePos, 0, 0));        // glTranslatef
  //glm::mat4 rotateRectangle = glm::rotate((float)(-rectangle_rotation*M_PI/18000.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
  Matrices.model *= (translateB );
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
  draw3DObject(blue);

 Matrices.model = glm::mat4(1.0f);
 

 glm::mat4 translateG = glm::translate (glm::vec3(greenPos, 0, 0));        // glTranslatef
  //glm::mat4 rotateRectangle = glm::rotate((float)(-rectangle_rotation*M_PI/18000.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
  Matrices.model *= (translateG );
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
  draw3DObject(green);
  
  // Load identity to model matrix
  Matrices.model = glm::mat4(1.0f);
 

  glm::mat4 translateRectangle = glm::translate (glm::vec3(0, -fall, 0));        // glTranslatef
  //glm::mat4 rotateRectangle = glm::rotate((float)(-rectangle_rotation*M_PI/18000.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
  Matrices.model *= (translateRectangle );
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
  draw3DObject(brick);

  

  Matrices.model = glm::mat4(1.0f);

  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
  if(laser)
    draw3DObject(laser);

  
  Matrices.model = glm::mat4(1.0f);

  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
 
  draw3DObject(mirror);

  Matrices.model = glm::mat4(1.0f);
 

  glm::mat4 translateRectangle1 = glm::translate (glm::vec3(-((cannonPos+0.05)*sin(cannonAngle*M_PI/180.0f)), (cannonPos+0.05)*cos(cannonAngle*M_PI/180.0f), 0));        // glTranslatef
  glm::mat4 translateRectangle2 = glm::translate (glm::vec3(4, -cannonPos-0.05, 0));  
  glm::mat4 rotateRectangle = glm::rotate((float)(cannonAngle*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
  glm::mat4 translateRectangle3 = glm::translate (glm::vec3(-4, cannonPos+0.05, 0)); 
  Matrices.model *= (translateRectangle1*translateRectangle3*rotateRectangle*translateRectangle2);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
  draw3DObject(cannon);

  // Increment angles
  float increments = 1;

}

/* Initialise glfw window, I/O callbacks and the renderer to use */
/* Nothing to Edit here */
GLFWwindow* initGLFW (int width, int height)
{
  GLFWwindow* window; // window desciptor/handle

  glfwSetErrorCallback(error_callback);
  if (!glfwInit()) {
    //        exit(EXIT_FAILURE);
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  window = glfwCreateWindow(width, height, "Sample OpenGL 3.3 Application", NULL, NULL);

  if (!window) {
    glfwTerminate();
    //        exit(EXIT_FAILURE);
  }

  glfwMakeContextCurrent(window);
  gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
  glfwSwapInterval( 1 );

  /* --- register callbacks with GLFW --- */

  /* Register function to handle window resizes */
  /* With Retina display on Mac OS X GLFW's FramebufferSize
     is different from WindowSize */
  glfwSetFramebufferSizeCallback(window, reshapeWindow);
  glfwSetWindowSizeCallback(window, reshapeWindow);

  /* Register function to handle window close */
  glfwSetWindowCloseCallback(window, quit);

  /* Register function to handle keyboard input */
  glfwSetKeyCallback(window, keyboard);      // general keyboard input
  glfwSetCharCallback(window, keyboardChar);  // simpler specific character handling

  /* Register function to handle mouse click */
  glfwSetMouseButtonCallback(window, mouseButton);  // mouse button clicks

  return window;
}

/* Initialize the OpenGL rendering properties */
/* Add all the models to be created here */
void initGL (GLFWwindow* window, int width, int height)
{
  /* Objects should be created before any other gl function and shaders */
  // Create the models
  createBrick(gen_point());
  createMirrors();
  createCannon();
  createBuckets();

  // Create and compile our GLSL program from the shaders
  programID = LoadShaders( "Sample_GL.vert", "Sample_GL.frag" );
  // Get a handle for our "MVP" uniform
  Matrices.MatrixID = glGetUniformLocation(programID, "MVP");

	
  reshapeWindow (window, width, height);

  // Background color of the scene
  glClearColor (1, 1, 1, 0.0f); // R, G, B, A
  glClearDepth (1.0f);

  glEnable (GL_DEPTH_TEST);
  glDepthFunc (GL_LEQUAL);

  cout << "VENDOR: " << glGetString(GL_VENDOR) << endl;
  cout << "RENDERER: " << glGetString(GL_RENDERER) << endl;
  cout << "VERSION: " << glGetString(GL_VERSION) << endl;
  cout << "GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
}

int main (int argc, char** argv)
{
 
  int width = 1000;
  int height = 1000;
  srand (time(NULL));
  last_update_time1 = glfwGetTime();
  GLFWwindow* window = initGLFW(width, height);

  initGL (window, width, height);
  //createMirrors();
  double last_update_time = glfwGetTime(), current_time, current_time1;

  /* Draw in loop */
  while (!glfwWindowShouldClose(window)&&!Over) {

    // OpenGL Draw commands
  

    //createBrick(gen_point());

    // Swap Frame Buffer in double buffering
    glfwSwapBuffers(window);

    // Poll for Keyboard and mouse events
    glfwPollEvents();

    // Control based on time (Time based transformation like 5 degrees rotation every 0.5s)
    current_time = glfwGetTime(); // Time in seconds
    if ((current_time - last_update_time) >= 0.5)  // atleast 0.5s elapsed since last frame
      // do something every 0.5 seconds ..
      {
	last_update_time = current_time;
	createBrick(gen_point());

      }

	
    current_time1 = glfwGetTime(); // Time in seconds
    if ((current_time1 - last_update_time1) >= 0.2&&Laser[0])  // atleast 0.5s elapsed since last frame
      // do something every 0.5 seconds ..
      {
	last_update_time1 = current_time1;

	for(int k=0;k<20;k++)Laser[k]=0;
	laser = create3DObject(GL_LINES,20, Laser, colLaser, GL_FILL);
      }

    fall+=speed;
 
      {checkBlue();checkGreen();}
    draw();

    
  }
    

         

  glfwTerminate();
  //    exit(EXIT_SUCCESS);
}
