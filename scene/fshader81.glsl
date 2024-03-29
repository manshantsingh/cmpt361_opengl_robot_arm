#version 150 

in vec3 normalInterp;
in vec3 vertPos;

in vec3 ambientColor;
in vec3 diffuseColor;
in vec3 specColor;


out vec4 fColor;

struct Light{
  vec3 pos;
  vec3 color;
  float power;
};

const int NUM_LIGHTS = 20;
Light lights[NUM_LIGHTS];

const vec3 LAMP_COLOR = vec3(1.0, 1.0, 0.3);
const float LAMP_POWER = 5.0;
const float LAMP_DISTANCE = 3.5;

// lights[0].pos = vec3(0, -50, -100);
// lights[0].color = vec3(1.0, 1.0, 1.0);
// lights[0].power = 10000.0;


// const vec3 ambientColor = vec3(0.1, 0.0, 0.0);
// const vec3 specColor = vec3(1.0, 1.0, 1.0);

const float shininess = 16.0;
const float screenGamma = 2.2; // Assume the monitor is calibrated to the sRGB color space

void main() 
{ 
  for(int i=0;i<NUM_LIGHTS;i+=2){
    lights[i] = Light(vec3(3.7, 4.5, -10 +i*LAMP_DISTANCE), LAMP_COLOR, LAMP_POWER);
    lights[i+1] = Light(vec3(-5.5, 4.5, -10.5 +i*LAMP_DISTANCE), LAMP_COLOR, LAMP_POWER);
  }
  // lights[0] = Light(vec3(0, 50, 100), vec3(1.0, 1.0, 1.0), 0.0);
  // lights[1] = Light(vec3(0, 50, -100), vec3(1.0, 1.0, 1.0), 0.0);
  // lights[2] = Light(vec3(3.7, 4.5, -11), vec3(1.0, 1.0, 0.3), 5.0);
  // lights[3] = Light(vec3(-5.0, 4.5, -11), vec3(1.0, 1.0, 0.3), 5.0);
  // lights[2] = Light(vec3(0, 50, 0), vec3(1.0, 1.0, 1.0), 0.0);
  // lights[2] = Light(vec3(0, 50, 0), vec3(1.0, 1.0, 1.0), 0.0);

    //msk
  // vec3 diffuseColor = vec3(color);
  vec3 normal = normalize(normalInterp);
  vec3 colorLinear = ambientColor;

  for(int i=0;i<NUM_LIGHTS;i++){
    vec3 lightPos = lights[i].pos;
    vec3 lightColor = lights[i].color;
    float lightPower = lights[i].power;

    vec3 lightDir = lightPos - vertPos;
    float distance = length(lightDir);
    distance = distance * distance;
    lightDir = normalize(lightDir);

    float lambertian = max(dot(lightDir,normal), 0.0);
    float specular = 0.0;

    if(lambertian > 0.0) {

      vec3 viewDir = normalize(-vertPos);

      // this is blinn phong
      vec3 halfDir = normalize(lightDir + viewDir);
      float specAngle = max(dot(halfDir, normal), 0.0);
      specular = pow(specAngle, shininess);
    }
    colorLinear = colorLinear + diffuseColor * lambertian * lightColor * lightPower / distance +
                       specColor * specular * lightColor * lightPower / distance;
  }
  // apply gamma correction (assume ambientColor, diffuseColor and specColor
  // have been linearized, i.e. have no gamma correction in them)
  vec3 colorGammaCorrected = pow(colorLinear, vec3(1.0/screenGamma));
  // use the gamma corrected color in the fragment
  fColor = vec4(colorGammaCorrected, 1.0);

  // fColor = vec4(diffuseColor, 1);
} 

