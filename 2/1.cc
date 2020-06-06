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
struct Cone{
  float h;
  float theta;
  vec3 center;
  vec3 norm;
  vec3 col;
};
struct Torus{
  float R;
  float r;
  vec3 center;
  vec3 norm;
  vec3 col;
};
//交点
struct Intersection{
  vec3 point; //rayと物体の交点の座標
  vec3 norm; //法線ベクトル
  vec3 col; //color
  float dist; //distance (これがないと、物体の前後関係がわからない)
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
    inter.hit = inter.hit + 1;
    inter.rayDir = ray.dir;
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
    inter.hit = inter.hit + 1;
    inter.rayDir = ray.dir;
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
    inter.hit = inter.hit + 1;
    inter.rayDir = ray.dir;
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

  //回転操作 + 平行移動
  //まず平行移動
  vec3 ori = ray.ori - cylinder.center;

  //次に回転移動
  vec3 axis = vec3(cylinder.norm[1],-cylinder.norm[0],0.0); //normに直交するz成分が0のベクトルを軸として回転すればいいのでそれを定義
  ori = rotate(ori,axis,sqrt(1.0-pow(normalize(cylinder.norm)[2],2.0)),normalize(cylinder.norm)[2]);
  vec3 dir = rotate(ray.dir,axis,sqrt(1.0-pow(normalize(cylinder.norm)[2],2.0)),normalize(cylinder.norm)[2]);

  //at^2+bt+cとして定義する
  //この時点では原点中心、高さhの円柱として見ている
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

  if(t < 100000000.0){
    //交点があった場合
    inter.point = ori + t * dir;
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

    //逆回転操作
    //回転操作
    //回転を行うべきは、inter.normとinter.pointだけ
    inter.point = rotate(inter.point,axis,-sqrt(1.0-pow(normalize(cylinder.norm)[2],2.0)),normalize(cylinder.norm)[2]) + cylinder.center;
    inter.norm = rotate(inter.norm,axis,-sqrt(1.0-pow(normalize(cylinder.norm)[2],2.0)),normalize(cylinder.norm)[2]);
    float d = clamp(dot(inter.norm, lightDir), 0.1, 1.0);
    inter.col = cylinder.col * d;
    inter.dist = t;
    inter.hit = inter.hit + 1;
    inter.rayDir = ray.dir;

  }
  return ;
}


void intersection_cone(Ray ray, Cone cone, inout Intersection inter){

  //回転操作 + 平行移動
  //まず平行移動
  vec3 ori = ray.ori - cone.center;

  //次に回転移動
  vec3 axis = vec3(cone.norm[1],-cone.norm[0],0.0); //normに直交するz成分が0のベクトルを軸として回転すればいいのでそれを定義
  ori = rotate(ori,axis,sqrt(1.0-pow(normalize(cone.norm)[2],2.0)),normalize(cone.norm)[2]);
  vec3 dir = rotate(ray.dir,axis,sqrt(1.0-pow(normalize(cone.norm)[2],2.0)),normalize(cone.norm)[2]);

  //at^2+bt+cとして定義する
  //この時点では原点中心、高さhの円錐として見ている
  float t = 100000000.0;
  float a = pow(dir[0],2.0) + pow(dir[1],2.0) - pow(dir[2]*tan(cone.theta),2.0);
  float b = 2.0 * (ori[0]*dir[0] + ori[1]*dir[1] + pow(tan(cone.theta),2.0) * dir[2] * (cone.h - ori[2]));
  float c = ori[0] * ori[0] + ori[1] * ori[1] - pow(tan(cone.theta) * (cone.h - ori[2]),2.0);
  float tempt;
  if(a > 0.0) tempt = (-b - sqrt(b * b - 4.0 * a * c))/ (2.0 * a); else (-b + sqrt(b * b - 4.0 * a * c))/ (2.0 * a);
  float tempt2 = - ori[2] / dir[2];
  if(tempt > 0. && ori[2] + dir[2] * tempt < cone.h && ori[2] + dir[2] * tempt > 0.) t = min(t,tempt);
  if(tempt2 > 0. && pow((ori[0] + tempt2 * dir[0]),2.0) + pow((ori[1] + tempt2 * dir[1]),2.0) < pow(tan(cone.theta) * cone.h,2.0)) t = min(t,tempt2);

  if(t < 100000000.0){
    //交点があった場合
    inter.point = ori + t * dir;
    //交点の場所によって法線ベクトルの場合わけを行う
    if(t == tempt){
      //円錐の側面の場合
      inter.norm = normalize(inter.point - vec3(0.0,0.0,inter.point[2]));
    }
    else {
      inter.norm = vec3(0.0,0.0,-1.0);
    }

    //逆回転操作
    //回転操作
    //回転を行うべきは、inter.normとinter.pointだけ
    inter.point = rotate(inter.point,axis,-sqrt(1.0-pow(normalize(cone.norm)[2],2.0)),normalize(cone.norm)[2]) + cone.center;
    inter.norm = rotate(inter.norm,axis,-sqrt(1.0-pow(normalize(cone.norm)[2],2.0)),normalize(cone.norm)[2]);
    float d = clamp(dot(inter.norm, lightDir), 0.1, 1.0);
    inter.col = cone.col * d;
    inter.dist = t;
    inter.hit = inter.hit + 1;
    inter.rayDir = ray.dir;

  }
  return ;
}


void intersection_torus(Ray ray, Torus torus, inout Intersection inter){

  //回転操作 + 平行移動
  //まず平行移動
  vec3 ori = ray.ori - torus.center;

  //次に回転移動
  vec3 axis = vec3(torus.norm[1],-torus.norm[0],0.0); //normに直交するz成分が0のベクトルを軸として回転すればいいのでそれを定義
  ori = rotate(ori,axis,sqrt(1.0-pow(normalize(torus.norm)[2],2.0)),normalize(torus.norm)[2]);
  vec3 dir = rotate(ray.dir,axis,sqrt(1.0-pow(normalize(torus.norm)[2],2.0)),normalize(torus.norm)[2]);


  //この時点では原点中心、のtorusとして見ている
  float t = 0.01;
  bool hit = false;

  for(int i = 0; i < 100; i++){
    //このループで交点に達する可動化を求める
    //torus とのdistanceを求める
    vec3 point = ori + t * dir;
    float dist = sqrt( pow((sqrt(point[0]*point[0] + point[1]*point[1]) - (torus.R - torus.r)),2.0) + pow(point[2],2.0)) - torus.r;
    if(dist < 0.0001) {hit = true; break;}
    t = t + dist;
  }


  if(hit){
    //交点があった場合
    inter.point = ori + t * dir;

    inter.norm = normalize(inter.point - (torus.R - torus.r) * normalize(vec3(inter.point[0],inter.point[1],0.0)));

    //逆回転操作
    //回転操作
    //回転を行うべきは、inter.normとinter.pointだけ
    inter.point = rotate(inter.point,axis,-sqrt(1.0-pow(normalize(torus.norm)[2],2.0)),normalize(torus.norm)[2]) + torus.center;
    inter.norm = rotate(inter.norm,axis,-sqrt(1.0-pow(normalize(torus.norm)[2],2.0)),normalize(torus.norm)[2]);
    float d = clamp(dot(inter.norm, lightDir), 0.1, 1.0);
    inter.col = torus.col * d;
    inter.dist = t;
    inter.hit = inter.hit + 1;
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

  //ray.ori = vec3(sin(mouse.x*rotspeed)*10.0, 2.0,cos(mouse*rotspeed)*10.0);
  //ray.dir = normalize(-ray.ori+vec3(cos(mouse.x*rotspeed),0.0,-sin(mouse.x*rotspeed))*pos.x*5.0 +vec3(0.0, 1.0, 0.0)*pos.y*5.0);


  //Sphereの定義
  Sphere sphere = Sphere(1.0, vec3(-1.0,0.0,-1.0),vec3(0.,0.8,23.3));

  //planeの定義
  Plane plane = Plane(vec3(0.0,-1.0,0.0), vec3(0.0,1.0,0.0), vec3(1.0));

  Ellipse ellipse = Ellipse(4.0, vec3(3.0,0.0,2.0),vec3(3.0,0.0,5.0),vec3(0.,0.8,23.3));

  Cylinder cylinder = Cylinder(1.0,5.0,vec3(-3.0,1.0,3.0),vec3(0.0,1.0,1.0),vec3(0.,0.8,23.3));

  Cone cone = Cone(3.0,0.4,vec3(-1.0,0.0,-5.0),vec3(1.0,0.0,1.0),vec3(0.0,0.8,23.3));

  Torus torus = Torus(2.0,0.5,vec3(0.0,3.0,-3.0),vec3(0.7,1.0,1.0),vec3(0.0,0.8,23.3));





  //intersectionの定義
  Intersection inter = Intersection(vec3(0.0),vec3(0.0),vec3(0.0),1.0e30);


  //sphereとのintersectionの計算
  intersection_sphere(ray,sphere,inter);
  intersection_plane(ray,plane,inter);
  intersection_ellipse(ray,ellipse,inter);
  intersection_cylinder(ray,cylinder,inter);
  intersection_cone(ray,cone,inter);
  intersection_torus(ray,torus,inter);

  // hit check
    vec3 destColor = vec3(ray.direction.y);
    vec3 tempColor = vec3(1.0);
    Ray q;
    intersectExec(ray, its);
    if(its.hit > 0){
        destColor = its.color;
        tempColor *= its.color;
        for(int j = 1; j < MAX_REF; j++){
            q.origin = its.hitPoint + its.normal * EPS;
            q.direction = reflect(its.rayDir, its.normal);
            intersectExec(q, its);
            if(its.hit > 0){
                destColor += tempColor * its.color;
                tempColor *= its.color;
            }
        }
    }

  //planeとのintersectionの計算

  gl_FragColor = vec4(inter.col, 1.0);
















}
