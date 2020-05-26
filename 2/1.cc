#ifdef GL_ES
precision mediump float;
#endif

#extension GL_OES_standard_derivatives : enable

uniform float time;
uniform vec2 mouse;
uniform vec2 resolution;

struct Ray{
  vec3 ori; //始点
  vec3 dir; //方向
};
//球
struct Sphere{
  float r;
  vec3 cen;
  vec3 col;
};
//交点
struct Intersection{
  vec3 point;
  vec3 norm;
  vec3 col;
  float dist;
  int hit;
  vec3 rayDir;
};


void intersection_sphere(Ray ray, Sphere sphere, inout Intersection inter){
  //交点があるかどうかを求める
  //二次方程式の部分をat^2 + bt + cとして変数をおく
  //ただし今回はvector{d}が正規化されていると考えて、a = 1である
  float b = dot(ray.ori-sphere.cen, ray.dir);
  float c = dot(ray.ori-sphere.cen,ray.ori-sphere.cen) - (sphere.r * sphere.r);
  float d = b * b - c;
  if(d > 0.0){
    //交点があった場合
    float t = -(b + sqrt(d));

    //struct intersectionを更新
    inter.point = ray.ori + ray.dir;
    inter.norm = normalize(inter.point - sphere.cen);
    inter.col = vec3(1.0,245,23);



  }

  return ;
}


void main( void ) {

  //vec2 pos = (fragCoord - 0.5*resolution.xy)/resolution.x;
  vec2 pos = (gl_FragCoord.xy * 2.0 - resolution)/min(resolution.x,resolution.y);
	
  // Ray の定義
  float rotspeed = 0.1;
  Ray ray;
  ray.ori = vec3(sin(time*rotspeed)*10.0, 2.0,cos(time*rotspeed)*10.0);
  //ray.dir = normalize(vec3(pos.x, pos.y, -1.0));
  ray.dir = normalize(-ray.ori+vec3(cos(time*rotspeed),0.0,-sin(time*rotspeed))*pos.x*5.0 +vec3(0.0, 1.0, 0.0)*pos.y*5.0);

  //Sphereの定義
  Sphere sphere = Sphere(1.0, vec3(-1.0,0.0,-1.0),vec3(1.0,1.0,1.0));

  //intersectionの定義
  Intersection inter = Intersection(vec3(0.0),vec3(0.0),vec3(0.0),1.0e30,0,vec3(0.0));
  intersection_sphere(ray,sphere,inter);

  gl_FragColor = vec4(inter.col, 1.0);
















}
