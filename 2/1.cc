#ifdef GL_ES
precision mediump float;
#endif

#extension GL_OES_standard_derivatives : enable

uniform float time;
uniform vec2 mouse;
uniform vec2 resolution;

const vec3 lightDir = vec3(0.777);

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
//平面
struct Plane{
  vec3 pos;
  vec3 norm;
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
  float t = 0.0;
  if(d > 0.0) t = -(b + sqrt(d));
  if(t > 0.0 && t < inter.dist){
    //交点があった場合
    //struct intersectionを更新
    inter.point = ray.ori + t * ray.dir;
    inter.norm = normalize(inter.point - sphere.cen);
    float d = clamp(dot(inter.norm, lightDir), 0.1, 1.0);
    inter.col = sphere.col * d;
    inter.dist = t;
  }
  return ;
}



void intersection_plane(Ray ray, Plane plane, inout Intersection inter){
  //ここでは平面を描画する
  float d = dot(ray.ori, plane.norm);
  float v = dot(ray.dir, plane.norm);
  float t = 0.0;
  if(v != 0.0) t = -(dot(ray.ori,plane.norm) + d)/v;

  if(t > 0.0 && t < inter.dist && d > 0.0){
    inter.point = ray.ori + ray.dir * t;
    inter.norm = plane.norm;
    //clampで0-1の範囲に収めている
    //lightの方向との内積をとる
    float d = clamp(dot(inter.norm, lightDir), 0.1, 1.0);
    float m = mod(inter.point.x, 5.0);
    float n = mod(inter.point.z, 5.0);
    if((m > 1.0 && n > 1.0) || (m < 1.0 && n < 1.0)){
      //こうすることでメッシュを描画する（このif文の中身にはいるとメッシュの色の濃い部分を描画する）
      d *= 0.5;
    }
    float f = 1.0 - min(abs(inter.point.z), 25.0) * 0.04;
    inter.col += plane.col * d ;
    inter.dist = t;
    inter.hit++;
    inter.rayDir = ray.dir;
  }

  return ;
}


void main( void ) {

  //vec2 pos = (fragCoord - 0.5*resolution.xy)/resolution.x;
  vec2 pos = (gl_FragCoord.xy * 2.0 - resolution)/min(resolution.x,resolution.y);
	
  // Ray の定義
  float rotspeed = 0.1;
  Ray ray;
  //固定のrayの設定を行う
  ray.ori = vec3(10.0, 2.0,.0);
  ray.dir = normalize(-ray.ori+vec3(0.0, 0.0, -1.0) *pos.x*(-5.0) +vec3(0.0, 1.0, 0.0)*pos.y*5.0);

  //回転するようなrayの設定を行う
  // ray.ori = vec3(sin(time*rotspeed)*10.0, 2.0,cos(time*rotspeed)*10.0);
  // ray.dir = normalize(-ray.ori+vec3(cos(time*rotspeed),0.0,-sin(time*rotspeed))*pos.x*5.0 +vec3(0.0, 1.0, 0.0)*pos.y*5.0);

  //Sphereの定義
  Sphere sphere = Sphere(2.0, vec3(-1.0,0.0,-1.0),vec3(0.,0.8,23.3));

  //planeの定義
  Plane plane = Plane(vec3(0.0,-1.0,0.0), vec3(0.0,1.0,0.0), vec3(1.0));

  //intersectionの定義
  Intersection inter = Intersection(vec3(0.0),vec3(0.0),vec3(0.0),1.0e30,0,vec3(0.0));


  //sphereとのintersectionの計算
  intersection_sphere(ray,sphere,inter);
  intersection_plane(ray,plane,inter);

  //planeとのintersectionの計算

  gl_FragColor = vec4(inter.col, 1.0);
















}
