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
struct Ellipse{
  //焦点がa,bでそこからの距離の和がrであるような楕円
  float r;
  vec3 a;
  vec3 b;
  vec3 col;
};
//平面
struct Plane{
  vec3 pos;
  vec3 norm;
  vec3 col;
};
struct Cylinder{
  float r;
  float h;
  vec3 center;
  vec3 norm;
  vec3 col; //color
};
//交点
struct Intersection{
  vec3 point; //rayと物体の交点の座標
  vec3 norm; //法線ベクトル
  vec3 col; //color
  float dist; //distance (これがないと、物体の前後関係がわからない)
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

void intersection_ellipse(Ray ray, Ellipse ellipse, inout Intersection inter){
  //交点があるかどうかを求める
  //二次方程式の部分をat^2 + bt + cとして変数をおく
  //ただし今回はvector{d}が正規化されていると考えて、a = 1である
  float a = 4.0 * dot(ray.dir,(ellipse.a - ellipse.b)) * dot(ray.dir,(ellipse.a - ellipse.b))
            - 4.0 * ellipse.r * ellipse.r * dot(ray.dir,ray.dir);

  float b = 4.0 * dot(ray.dir,(ellipse.a - ellipse.b)) *
            (dot(ray.ori - ellipse.b, ray.ori - ellipse.b) - dot(ray.ori - ellipse.a, ray.ori - ellipse.a) + ellipse.r * ellipse.r)
            - 4.0 * ellipse.r * ellipse.r * 2.0 * dot(ray.dir,(ray.ori - ellipse.b));

  float c = (dot(ray.ori - ellipse.b, ray.ori - ellipse.b) - dot(ray.ori - ellipse.a, ray.ori - ellipse.a) + ellipse.r * ellipse.r) *
            (dot(ray.ori - ellipse.b, ray.ori - ellipse.b) - dot(ray.ori - ellipse.a, ray.ori - ellipse.a) + ellipse.r * ellipse.r)
            - 4.0 * ellipse.r * ellipse.r * dot(ray.ori - ellipse.b, ray.ori - ellipse.b);

  float d = b * b - 4.0 * a * c;
  float t = 0.0;
  if(d > 0.0) t = -(b - sqrt(d))/ (2.0 * a);
  if(t > 0.0 && t < inter.dist){
    //交点があった場合
    //struct intersectionを更新
    inter.point = ray.ori + t * ray.dir;
    float a2 = ellipse.r / 2.0;
    float b2 = sqrt((ellipse.r/2.0) * (ellipse.r/2.0) - ((ellipse.a[2] - ellipse.b[2])/2.0) * ((ellipse.a[2] - ellipse.b[2])/2.0));
    vec3 nor = inter.point - (ellipse.b + ellipse.a)/2.0;
    inter.norm = normalize(vec3(nor[0]*a2,nor[1]*a2,nor[2]*b2));
    inter.norm = normalize(nor);
    float d = clamp(dot(inter.norm, lightDir), 0.1, 1.0);
    inter.col = ellipse.col * d;
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
  }

  return ;
}

vec3 rotate(vec3 p, vec3 axis, float s, float c){
  //axisの軸に対して、vec3 pを角度angle分回転させる関数
    vec3 a = normalize(axis);
    //float s = sin(angle);
    //float c = cos(angle);
    float r = 1.0 - c;
    mat3 m = mat3(
        a.x * a.x * r + c,
        a.y * a.x * r + a.z * s,
        a.z * a.x * r - a.y * s,
        a.x * a.y * r - a.z * s,
        a.y * a.y * r + c,
        a.z * a.y * r + a.x * s,
        a.x * a.z * r + a.y * s,
        a.y * a.z * r - a.x * s,
        a.z * a.z * r + c
    );
    return m * p;
}

void intersection_cylinder(Ray ray, Cylinder cylinder, inout Intersection inter){
  /*
  struct Cylinder{
    float r;
    float h;
    vec3 center;
    vec3 norm;
    vec3 col; //color
  };
  */
  //TODO: あとは回転させるだけ
  
  //回転操作 + 平行移動
  //まず平行移動
  vec3 ori = ray.ori - cylinder.center;

  //次に回転移動
  vec3 axis = vec3(cylinder.norm[1],-cylinder.norm[0],0.0); //normに直交するz成分が0のベクトルを軸として回転すればいいのでそれを定義
  vec3 dir = rotate(ray.dir,axis,sqrt(1.0-pow(cylinder.norm[2],2.0)),cylinder.norm[2]);

  float t = 100000000.0;
  float a = dir[0] * dir[0] + dir[1] * dir[1];
  float b = 2.0 * (dir[0] * ori[0] + dir[1] * ori[1]);
  float c = ori[0] * ori[0] + ori[1] * ori[1] - cylinder.r * cylinder.r;
  float tempt = (-b - sqrt(b * b - 4.0 * a * c))/ (2.0 * a);
  float tempt2 = (cylinder.h - ori[2]) / dir[2];
  float tempt3 = - ori[2] / dir[2];
  if(tempt > 0. && ori[2] + dir[2] * tempt < cylinder.h && ori[2] + dir[2] * tempt > 0.) t = min(t,tempt);
  if(tempt2 > 0. && pow((ori[0] + tempt2 * dir[0]),2.0) + pow((ori[1] + tempt2 * dir[1]),2.0) < cylinder.r * cylinder.r) t = min(t,tempt2);
  if(tempt3 > 0. && pow((ori[0] + tempt3 * dir[0]),2.0) + pow((ori[1] + tempt3 * dir[1]),2.0) < cylinder.r * cylinder.r) t = min(t,tempt3);

  
  //at^2+bt+cとして定義する
  //この時点では原点中心、高さhの円柱として見ている
  // float t = 100000000.0;
  // float a = ray.dir[0] * ray.dir[0] + ray.dir[1] * ray.dir[1];
  // float b = 2.0 * (ray.dir[0] * ray.ori[0] + ray.dir[1] * ray.ori[1]);
  // float c = ray.ori[0] * ray.ori[0] + ray.ori[1] * ray.ori[1] - cylinder.r * cylinder.r;
  // float tempt = (-b - sqrt(b * b - 4.0 * a * c))/ (2.0 * a);
  // float tempt2 = (cylinder.h - ray.ori[2]) / ray.dir[2];
  // float tempt3 = - ray.ori[2] / ray.dir[2];
  // if(tempt > 0. && ray.ori[2] + ray.dir[2] * tempt < cylinder.h && ray.ori[2] + ray.dir[2] * tempt > 0.) t = min(t,tempt);
  // if(tempt2 > 0. && pow((ray.ori[0] + tempt2 * ray.dir[0]),2.0) + pow((ray.ori[1] + tempt2 * ray.dir[1]),2.0) < cylinder.r * cylinder.r) t = min(t,tempt2);
  // if(tempt3 > 0. && pow((ray.ori[0] + tempt3 * ray.dir[0]),2.0) + pow((ray.ori[1] + tempt3 * ray.dir[1]),2.0) < cylinder.r * cylinder.r) t = min(t,tempt3);
  if(t < 100000000.0){
    //交点があった場合
    inter.point = ray.ori + t * ray.dir;
    //交点の場所によって法線ベクトルの場合わけを行う
    if(t == tempt){
      //円柱の側面の場合
      inter.norm = normalize(inter.point - vec3(0.0,0.0,inter.point[2]));
    }
    else if(t == tempt2){
      inter.norm = vec3(0.0,0.0,1.0);
    }
    else{
      inter.norm = vec3(0.0,0.0,-1.0);
    }
    float d = clamp(dot(inter.norm, lightDir), 0.1, 1.0);
    inter.col = cylinder.col * d;
    
    //逆回転操作
    //回転操作
    //回転を行うべきは、inter.normとinter.pointだけ
    inter.point = rotate(inter.point,axis,-sqrt(1.0-pow(cylinder.norm[2],2.0)),cylinder.norm[2]);
    inter.point = inter.point + cylinder.center;
    inter.col = cylinder.col;


    
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

  //ray.ori = vec3(sin(mouse.x*rotspeed)*10.0, 2.0,cos(mouse*rotspeed)*10.0);
  //ray.dir = normalize(-ray.ori+vec3(cos(mouse.x*rotspeed),0.0,-sin(mouse.x*rotspeed))*pos.x*5.0 +vec3(0.0, 1.0, 0.0)*pos.y*5.0);


  //Sphereの定義
  Sphere sphere = Sphere(2.0, vec3(-1.0,0.0,-1.0),vec3(0.,0.8,23.3));

  //planeの定義
  Plane plane = Plane(vec3(0.0,-1.0,0.0), vec3(0.0,1.0,0.0), vec3(1.0));

  Ellipse ellipse = Ellipse(4.0, vec3(-1.0,0.0,3.0),vec3(-1.0,0.0,6.0),vec3(0.,0.8,23.3));

  Cylinder cylinder = Cylinder(1.0,3.0,vec3(-1.0,0.0,2.0),vec3(-1.0,0.0,1.0),vec3(0.,0.8,23.3));
  /*
  struct Cylinder{
    float r;
    float h;
    vec3 center;
    vec3 norm;
    vec3 col; //color
  };
  */

  //intersectionの定義
  Intersection inter = Intersection(vec3(0.0),vec3(0.0),vec3(0.0),1.0e30);


  //sphereとのintersectionの計算
  intersection_sphere(ray,sphere,inter);
  intersection_plane(ray,plane,inter);
  intersection_ellipse(ray,ellipse,inter);
  intersection_cylinder(ray,cylinder,inter);

  //planeとのintersectionの計算

  gl_FragColor = vec4(inter.col, 1.0);
















}
