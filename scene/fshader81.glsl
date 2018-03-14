#version 150 

in  vec4 color;
in vec3 normalInterp;
in vec3 vertPos;

out vec4 fColor;


const vec3 lightPos = vec3(0, -5, -25);
const vec3 lightColor = vec3(1.0, 1.0, 1.0);
const float lightPower = 1000.0;

const vec3 ambientColor = vec3(0.1, 0.0, 0.0);
// const vec3 diffuseColor = vec3(0.5, 0.0, 0.0);
const vec3 specColor = vec3(1.0, 1.0, 1.0);

const float shininess = 16.0;
const float screenGamma = 2.2; // Assume the monitor is calibrated to the sRGB color space

void main() 
{ 
    // fColor = color;
    // vec3 uLightDirection = vec3(0,0,1); //changeme
    // vec4 uMaterialDiffuse = color;


    // vec3 L = normalize(uLightDirection);
    // vec3 N = normalize(normalInterp);
     
    //  //Lambert's cosine law
    //  float lambertTerm = dot(N,-L);
     
    //  //Ambient Term
    //  vec4 Ia = vec4(lightAmbientColor * ambientColor, 1.0);
     
    //  //Diffuse Term
    //  vec4 Id = vec4(0.0,0.0,0.0,1.0);
     
    //  //Specular Term
    //  vec4 Is = vec4(0.0,0.0,0.0,1.0);
     
    //  if(lambertTerm > 0.0) //only if lambertTerm is positive
    //  {
    //       Id = vec4(lightDiffuseColor,1) * uMaterialDiffuse * lambertTerm; //add diffuse term
          
    //       vec3 E = normalize(-vertPos);
    //       vec3 R = reflect(L, N);
    //       float specular = pow( max(dot(R, E), 0.0), shininess);
          
    //       Is = vec4(lightSpecColor * specColor,1) * specular; //add specular term 
    //  }
     
    //  //Final color
    //  vec4 finalColor = Ia + Id + Is;
    //  finalColor.a = 1.0;
     
    //  fColor = finalColor;





    // vec3 L = normalize(lightPos - vertPos);
    // vec3 E = normalize(-vertPos);
    // vec3 R = normalize(-reflect(L,normalInterp));

    // vec4 Iamb = vec4(ambientColor, 1.0);
    // vec4 Idiff = vec4(diffuseColor * max(dot(normalInterp,L), 0.0), 1.0);
    // Idiff = clamp(Idiff, 0.0, 1.0);

    // vec4 Ispec = vec4(specColor, 1.0) * pow(max(dot(R,E),0.0),0.3*shininess);
    // Ispec = clamp(Ispec, 0.0, 1.0);

    // fColor = color + Iamb + Idiff + Ispec;


    //msk
  vec3 diffuseColor = vec3(color);
  vec3 normal = normalize(normalInterp);
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
       
    // // this is phong (for comparison)
    // if(mode == 2) {
    //   vec3 reflectDir = reflect(-lightDir, normal);
    //   specAngle = max(dot(reflectDir, viewDir), 0.0);
    //   // note that the exponent is different here
    //   specular = pow(specAngle, shininess/4.0);
    // }
  }
  vec3 colorLinear = ambientColor +
                     diffuseColor * lambertian * lightColor * lightPower / distance +
                     specColor * specular * lightColor * lightPower / distance;
  // apply gamma correction (assume ambientColor, diffuseColor and specColor
  // have been linearized, i.e. have no gamma correction in them)
  vec3 colorGammaCorrected = pow(colorLinear, vec3(1.0/screenGamma));
  // use the gamma corrected color in the fragment
  fColor = vec4(colorGammaCorrected, 1.0);

  // fColor = color;
} 

