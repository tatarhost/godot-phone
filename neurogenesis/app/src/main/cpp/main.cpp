#include <android_native_app_glue.h>
#include <android/log.h>
#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <btBulletDynamicsCommon.h>
#include <array>
#include <vector>
#include <string>
#include <cmath>
#include <random>
#include <algorithm>
#include <cstdio>

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,"Neurogenesis",__VA_ARGS__)
static const char* VS = R"(#version 300 es
layout(location=0) in vec3 aPos;
uniform mat4 uMVP;
void main(){ gl_Position=uMVP*vec4(aPos,1.0); })";
static const char* FS = R"(#version 300 es
precision mediump float;
uniform vec4 uColor;
out vec4 frag;
void main(){ frag=uColor; })";

struct V3 { float x,y,z; };
static const float cube[] = {
-.5f,-.5f,-.5f, .5f,-.5f,-.5f, .5f,.5f,-.5f, -.5f,-.5f,-.5f, .5f,.5f,-.5f, -.5f,.5f,-.5f,
-.5f,-.5f,.5f, .5f,.5f,.5f, .5f,-.5f,.5f, -.5f,-.5f,.5f, -.5f,.5f,.5f, .5f,.5f,.5f,
-.5f,-.5f,-.5f, -.5f,.5f,-.5f, -.5f,.5f,.5f, -.5f,-.5f,-.5f, -.5f,.5f,.5f, -.5f,-.5f,.5f,
.5f,-.5f,-.5f, .5f,-.5f,.5f, .5f,.5f,.5f, .5f,-.5f,-.5f, .5f,.5f,.5f, .5f,.5f,-.5f,
-.5f,-.5f,-.5f, -.5f,-.5f,.5f, .5f,-.5f,.5f, -.5f,-.5f,-.5f, .5f,-.5f,.5f, .5f,-.5f,-.5f,
-.5f,.5f,-.5f, .5f,.5f,.5f, -.5f,.5f,.5f, -.5f,.5f,-.5f, .5f,.5f,-.5f, .5f,.5f,.5f
};

static GLuint compile(GLenum type,const char* src){
 GLuint s=glCreateShader(type); glShaderSource(s,1,&src,nullptr); glCompileShader(s);
 GLint ok=0; glGetShaderiv(s,GL_COMPILE_STATUS,&ok); if(!ok){char log[512];glGetShaderInfoLog(s,512,nullptr,log);LOGI("shader: %s",log);}
 return s;
}
static GLuint program(){
 GLuint p=glCreateProgram(); glAttachShader(p,compile(GL_VERTEX_SHADER,VS)); glAttachShader(p,compile(GL_FRAGMENT_SHADER,FS)); glLinkProgram(p);
 GLint ok=0;glGetProgramiv(p,GL_LINK_STATUS,&ok);if(!ok)LOGI("program link failed");return p;
}
static void identity(float*m){std::fill(m,m+16,0.f);m[0]=m[5]=m[10]=m[15]=1.f;}
static void multiply(float*out,const float*a,const float*b){
 for(int c=0;c<4;c++)for(int r=0;r<4;r++){out[c*4+r]=0;for(int k=0;k<4;k++)out[c*4+r]+=a[k*4+r]*b[c*4+k];}
}
static void perspective(float*m,float fovy,float aspect,float n,float f){
 std::fill(m,m+16,0.f);float q=1.f/std::tan(fovy*.5f);m[0]=q/aspect;m[5]=q;m[10]=(f+n)/(n-f);m[11]=-1;m[14]=(2*f*n)/(n-f);
}
static void lookAt(float*m,float ex,float ey,float ez){
 float fx=-ex,fy=-ey,fz=-ez,fl=std::sqrt(fx*fx+fy*fy+fz*fz);fx/=fl;fy/=fl;fz/=fl;
 float sx=-fz,sy=0,sz=fx,sl=std::sqrt(sx*sx+sz*sz);sx/=sl;sz/=sl;
 float ux=sy*fz-sz*fy,uy=sz*fx-sx*fz,uz=sx*fy-sy*fx;
 identity(m);m[0]=sx;m[4]=sy;m[8]=sz;m[1]=ux;m[5]=uy;m[9]=uz;m[2]=-fx;m[6]=-fy;m[10]=-fz;
 m[12]=-(sx*ex+sy*ey+sz*ez);m[13]=-(ux*ex+uy*ey+uz*ez);m[14]=fx*ex+fy*ey+fz*ez;
}

class Brain {
 std::vector<float>w; std::mt19937 rng;
public:
 static constexpr int Inputs=8, Hidden=12, Outputs=4;
 Brain():w((Inputs*Hidden+Hidden)+(Hidden*Outputs+Outputs)),rng(std::random_device{}()){mutate(1.f);}
 std::array<float,Outputs> think(const std::array<float,Inputs>&in){
  std::array<float,Hidden>h{};size_t k=0;
  for(int j=0;j<Hidden;j++){float s=w[k++];for(int i=0;i<Inputs;i++)s+=in[i]*w[k++];h[j]=std::tanh(s);}
  std::array<float,Outputs>o{};for(int j=0;j<Outputs;j++){float s=w[k++];for(int i=0;i<Hidden;i++)s+=h[i]*w[k++];o[j]=std::tanh(s);}return o;
 }
 void mutate(float amount){std::normal_distribution<float>d(0.f,amount);for(float&v:w)v=std::clamp(v+d(rng),-3.f,3.f);}
 void crossover(const Brain&a,const Brain&b){std::uniform_int_distribution<int>d(0,1);for(size_t i=0;i<w.size();i++)w[i]=d(rng)?a.w[i]:b.w[i];mutate(.18f);}
};

struct Sim {
 btDefaultCollisionConfiguration config;
 btCollisionDispatcher dispatcher{&config};
 btDbvtBroadphase broadphase;
 btSequentialImpulseConstraintSolver solver;
 btDiscreteDynamicsWorld world{&dispatcher,&broadphase,&solver,&config};
 std::vector<btCollisionShape*> shapes;
 std::vector<btRigidBody*> bodies;
 std::vector<btTypedConstraint*> joints;
 Brain brain;
 int generation=1, episode=0;
 float elapsed=0, best=0, startX=0;
 bool training=true;
 std::mt19937 rng{std::random_device{}()};
 Sim(){world.setGravity(btVector3(0,-9.81f,0));world.getSolverInfo().m_numIterations=8;resetCreature();}
 ~Sim(){clear();}
 void clear(){for(auto*j:joints){world.removeConstraint(j);delete j;}joints.clear();for(auto*b:bodies){world.removeRigidBody(b);delete b->getMotionState();delete b;}bodies.clear();for(auto*s:shapes)delete s;shapes.clear();}
 btRigidBody* box(float x,float y,float z,float mass,const btVector3&p){
  auto*s=new btBoxShape(btVector3(x*.5f,y*.5f,z*.5f));shapes.push_back(s);
  btTransform t;t.setIdentity();t.setOrigin(p);btVector3 inertia(0,0,0);if(mass>0)s->calculateLocalInertia(mass,inertia);
  auto*ms=new btDefaultMotionState(t);btRigidBody::btRigidBodyConstructionInfo ci(mass,ms,s,inertia);auto*b=new btRigidBody(ci);
  b->setActivationState(DISABLE_DEACTIVATION);b->setFriction(.9f);world.addRigidBody(b);bodies.push_back(b);return b;
 }
 void hinge(btRigidBody*a,btRigidBody*b,const btVector3&pa,const btVector3&pb){
  auto*j=new btHingeConstraint(*a,*b,pa,pb,btVector3(0,0,1),btVector3(0,0,1),true);
  j->setLimit(-.9f,.9f);world.addConstraint(j,true);joints.push_back(j);
 }
 void resetCreature(){
  clear();
  box(30,.5f,30,0,btVector3(0,-.35f,0));
  auto*torso=box(.9f,.42f,.42f,3,btVector3(0,1.2f,0));
  auto*head=box(.32f,.32f,.32f,.6f,btVector3(.62f,1.27f,0));
  auto*left=box(.22f,.65f,.22f,.7f,btVector3(-.28f,.68f,0));
  auto*right=box(.22f,.65f,.22f,.7f,btVector3(.28f,.68f,0));
  hinge(torso,left,btVector3(-.28f,-.2f,0),btVector3(0,.3f,0));
  hinge(torso,right,btVector3(.28f,-.2f,0),btVector3(0,.3f,0));
  auto*armL=box(.16f,.48f,.16f,.35f,btVector3(-.58f,1.05f,0));
  auto*armR=box(.16f,.48f,.16f,.35f,btVector3(.02f,1.05f,0));
  hinge(torso,armL,btVector3(-.38f,.1f,0),btVector3(0,.2f,0));
  hinge(torso,armR,btVector3(.38f,.1f,0),btVector3(0,.2f,0));
  (void)head;startX=torso->getWorldTransform().getOrigin().x();elapsed=0;
 }
 void step(float dt){
  if(training){
   auto a=joints.size()>0?static_cast<btHingeConstraint*>(joints[0])->getHingeAngle():0.f;
   auto b=joints.size()>1?static_cast<btHingeConstraint*>(joints[1])->getHingeAngle():0.f;
   auto c=joints.size()>2?static_cast<btHingeConstraint*>(joints[2])->getHingeAngle():0.f;
   auto d=joints.size()>3?static_cast<btHingeConstraint*>(joints[3])->getHingeAngle():0.f;
   std::array<float,Inputs> in={a,b,c,d,0.f,0.f,1.f,std::sin(elapsed*2.f)};
   auto out=brain.think(in);
   for(size_t i=0;i<joints.size()&&i<out.size();i++)static_cast<btHingeConstraint*>(joints[i])->enableAngularMotor(true,out[i]*5.f,2.2f);
  }
  world.stepSimulation(dt,2,1.f/60.f);elapsed+=dt;
  if(elapsed>12.f){
   float x=bodies.size()>1?bodies[1]->getWorldTransform().getOrigin().x():0.f;
   float fitness=x-startX+1.f/(1.f+std::abs(bodies[1]->getWorldTransform().getOrigin().y()-1.f));
   best=std::max(best,fitness);episode++;generation++;
   brain.mutate(.16f);resetCreature();
  }
 }
};

struct App {
 android_app* app{}; EGLDisplay display=EGL_NO_DISPLAY; EGLSurface surface=EGL_NO_SURFACE; EGLContext context=EGL_NO_CONTEXT;
 int width=0,height=0;GLuint prog=0,vbo=0;GLint mvpLoc=-1,colorLoc=-1;Sim sim;bool ready=false;
 bool init(){
  const EGLint attrs[]={EGL_RENDERABLE_TYPE,EGL_OPENGL_ES3_BIT,EGL_SURFACE_TYPE,EGL_WINDOW_BIT,EGL_BLUE_SIZE,8,EGL_GREEN_SIZE,8,EGL_RED_SIZE,8,EGL_NONE};
  display=eglGetDisplay(EGL_DEFAULT_DISPLAY);if(display==EGL_NO_DISPLAY||!eglInitialize(display,nullptr,nullptr))return false;
  EGLConfig cfg;EGLint count;eglChooseConfig(display,attrs,&cfg,1,&count);if(!count)return false;
  const EGLint ca[]={EGL_CONTEXT_CLIENT_VERSION,3,EGL_NONE};context=eglCreateContext(display,cfg,EGL_NO_CONTEXT,ca);
  surface=eglCreateWindowSurface(display,cfg,app->window,nullptr);if(context==EGL_NO_CONTEXT||surface==EGL_NO_SURFACE)return false;
  if(!eglMakeCurrent(display,surface,surface,context))return false;
  eglQuerySurface(display,surface,EGL_WIDTH,&width);eglQuerySurface(display,surface,EGL_HEIGHT,&height);
  prog=program();glGenBuffers(1,&vbo);glBindBuffer(GL_ARRAY_BUFFER,vbo);glBufferData(GL_ARRAY_BUFFER,sizeof(cube),cube,GL_STATIC_DRAW);
  mvpLoc=glGetUniformLocation(prog,"uMVP");colorLoc=glGetUniformLocation(prog,"uColor");
  glEnable(GL_DEPTH_TEST);glViewport(0,0,width,height);ready=true;return true;
 }
 void drawBody(btRigidBody*b,const float*vp,const float color[4],float sx=1,float sy=1,float sz=1){
  btTransform t;b->getMotionState()->getWorldTransform(t);btScalar m[16];t.getOpenGLMatrix(m);
  float model[16];for(int i=0;i<16;i++)model[i]=m[i];model[0]*=sx;model[1]*=sx;model[2]*=sx;model[4]*=sy;model[5]*=sy;model[6]*=sy;model[8]*=sz;model[9]*=sz;model[10]*=sz;
  float out[16];multiply(out,vp,model);glUniformMatrix4fv(mvpLoc,1,GL_FALSE,out);glUniform4fv(colorLoc,1,color);glDrawArrays(GL_TRIANGLES,0,36);
 }
 void render(){
  if(!ready)return;glClearColor(.055f,.07f,.08f,1);glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
  float p[16],v[16],vp[16];perspective(p,0.78f,(float)width/(float)height,.1f,100.f);lookAt(v,3.7f,2.7f,5.2f);multiply(vp,p,v);
  glUseProgram(prog);glBindBuffer(GL_ARRAY_BUFFER,vbo);glEnableVertexAttribArray(0);glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,0,(void*)0);
  const float floorC[]={.22f,.27f,.28f,1},bodyC[]={.83f,.88f,.86f,1},jointC[]={.28f,.68f,.62f,1};
  // Floor and articulated bodies are rendered from their live Bullet transforms.
  float floorM[16];identity(floorM);floorM[0]=15;floorM[5]=.04f;floorM[10]=15;floorM[13]=-.35f;
  float fm[16];multiply(fm,vp,floorM);glUniformMatrix4fv(mvpLoc,1,GL_FALSE,fm);glUniform4fv(colorLoc,1,floorC);glDrawArrays(GL_TRIANGLES,0,36);
  for(size_t i=1;i<sim.bodies.size();i++)drawBody(sim.bodies[i],vp,i>1?bodyC:jointC,i==1?.9f:(i==2?.32f:.22f),i==1?.42f:(i==2?.32f:(i<4?.65f:.48f)),i==1?.42f:.2f);
  glDisableVertexAttribArray(0);eglSwapBuffers(display,surface);
 }
 void shutdown(){ready=false;if(display!=EGL_NO_DISPLAY){eglMakeCurrent(display,EGL_NO_SURFACE,EGL_NO_SURFACE,EGL_NO_CONTEXT);if(context!=EGL_NO_CONTEXT)eglDestroyContext(display,context);if(surface!=EGL_NO_SURFACE)eglDestroySurface(display,surface);eglTerminate(display);}display=EGL_NO_DISPLAY;surface=EGL_NO_SURFACE;context=EGL_NO_CONTEXT;}
};

static void handle_cmd(android_app*app,int32_t cmd){
 auto*a=(App*)app->userData;
 if(cmd==APP_CMD_INIT_WINDOW&&app->window){if(!a->init())LOGI("EGL init failed");}
 else if(cmd==APP_CMD_TERM_WINDOW)a->shutdown();
}
static int32_t handle_input(android_app*app,AInputEvent*event){
 if(AInputEvent_getType(event)==AINPUT_EVENT_TYPE_MOTION&&AMotionEvent_getAction(event)==AMOTION_EVENT_ACTION_UP){
  auto*a=(App*)app->userData;a->sim.training=!a->sim.training;return 1;
 }return 0;
}
void android_main(android_app*state){
 app_dummy();App app;app.app=state;state->userData=&app;state->onAppCmd=handle_cmd;state->onInputEvent=handle_input;
 while(true){
  int events;android_poll_source*source;
  while(ALooper_pollAll(app.ready?0: -1,nullptr,&events,(void**)&source)>=0){if(source)source->process(state,source);if(state->destroyRequested){app.shutdown();return;}}
  if(app.ready){app.sim.step(1.f/60.f);app.render();}
 }
}
