#include <iostream> // Se incluye libreria para manejo de entrada/salida
#include <vector> // Se incluye libreria para manejo de vectores
#include <cmath> // Se incluye libreria para funciones matematicas
#include <GLFW/glfw3.h> 
#define STB_IMAGE_IMPLEMENTATION // Definición para implementación de stb_image
#include "include/stb_image.h" // Se incluye libreria para carga de imagenes

// --- VARIABLES GLOBALES ---
const unsigned int ANCHO_VENTANA = 800;
const unsigned int ALTO_VENTANA = 600;

// Cámara (Controlada ahora por Teclado)
float camYaw = 45.0f;
float camPitch = 20.0f;
float camDist = 10.0f;

// Animación Respiración
float factorRespiracion = 1.0f;

// ESTADO: Transparencia
bool transPulmonIzq = false;
bool transPulmonDer = false;
// Visibilidad de Costillas
bool mostrarCostillas = true;
bool tecla3Presionada = false;
// Control de teclas (rebote)
bool tecla1Presionada = false;
bool tecla2Presionada = false;

// Textura
GLuint texturaPulmonID = 0;

// --- CARGA DE TEXTURAS ---
bool cargarTextura(const char* ruta)
{
	// Cargar imagen usando stb_image
    int ancho, alto, canales;
    unsigned char* data = stbi_load(ruta, &ancho, &alto, &canales, 0);
    if (!data)
    {
        std::cerr << "Error al cargar la imagen: " << ruta << "\n";
        return false;
    }
	// Crear textura OpenGL
    glGenTextures(1, &texturaPulmonID);
    glBindTexture(GL_TEXTURE_2D, texturaPulmonID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	// imagen cargada en OpenGL
    GLenum formato = (canales == 4) ? GL_RGBA : GL_RGB;
    glTexImage2D(GL_TEXTURE_2D, 0, formato, ancho, alto, 0, formato, GL_UNSIGNED_BYTE, data);
	// Liberar memoria de la imagen cargada
    stbi_image_free(data);
    return true;
}

// --- CONFIGURACIÓN OPENGL ---
bool inicializarOpenGL()
{
	// Configuraciones básicas
	glEnable(GL_DEPTH_TEST);// Habilitar prueba de profundidad
	glEnable(GL_LIGHTING);// Habilitar iluminación
	glEnable(GL_LIGHT0);// Habilitar luz 0
	glEnable(GL_COLOR_MATERIAL);// Habilitar material por color
	glEnable(GL_NORMALIZE);// Habilitar normalización de normales
	glEnable(GL_BLEND);// Habilitar blending para transparencia
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);// Función de mezcla para transparencia
    
	glClearColor(0.05f, 0.05f, 0.08f, 1.0f);// Color de fondo

    // Luces
	GLfloat posicionLuz[] = { 4.0f, 6.0f, 5.0f, 1.0f };// Posición de la luz
	glLightfv(GL_LIGHT0, GL_POSITION, posicionLuz);// Establecer posición de la luz
	// Componentes de la luz
	GLfloat luzAmbiente[] = { 0.3f, 0.3f, 0.3f, 1.0f };// Componente ambiental
	GLfloat luzDifusa[] = { 0.9f, 0.9f, 0.9f, 1.0f };// Componente difusa
	GLfloat luzEspecular[] = { 0.3f, 0.3f, 0.3f, 1.0f };// Componente especular
	// Configurar luz 0
	glLightfv(GL_LIGHT0, GL_AMBIENT, luzAmbiente);// Establecer componente ambiental
	glLightfv(GL_LIGHT0, GL_DIFFUSE, luzDifusa);// Establecer componente difusa
	glLightfv(GL_LIGHT0, GL_SPECULAR, luzEspecular);// Establecer componente especular
	// Materiales
	GLfloat matEspecular[] = { 0.4f, 0.4f, 0.4f, 1.0f };// Componente especular del material
	GLfloat shininess[] = { 32.0f };// Brillo del material
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, matEspecular);// Establecer componente especular del material
	glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, shininess);// Establecer brillo del material
	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);// Configurar material por color
	// Cargar textura
    stbi_set_flip_vertically_on_load(true);
    if (!cargarTextura("textures/pulmon1.jpg"))
    {
        return false;
    }
	return true;// Éxito
}

// --- PROYECCIÓN ---
void configurarProyeccion(int ancho, int alto)
{
	// Evitar división por cero
	if (alto == 0) alto = 1;// Prevenir división por cero
    glViewport(0, 0, ancho, alto);//
	glMatrixMode(GL_PROJECTION);// Seleccionar matriz de proyección
	glLoadIdentity();// Cargar matriz identidad
	// Configurar proyección en perspectiva
	float aspect = (float)ancho / (float)alto;// Aspect ratio
	float fovY = 45.0f;// Campo de visión vertical
	float nearPlane = 0.1f;// Plano cercano
	float farPlane = 100.0f;// Plano lejano
	// Cálculo de los parámetros del frustum
	float top = nearPlane * tanf(fovY * 3.14159265f / 360.0f);// Cálculo de top
	float bottom = -top;// Cálculo de bottom
	float right = top * aspect;// Cálculo de right
	float left = -right;// Cálculo de left
	// Aplicar frustum
	glFrustum(left, right, bottom, top, nearPlane, farPlane);// Configurar frustum
	glMatrixMode(GL_MODELVIEW);// Seleccionar matriz de modelo-vista
	glLoadIdentity();// Cargar matriz identidad
}

// --- INPUT (TECLADO) ---
void callbackCambioTamano(GLFWwindow* ventana, int ancho, int alto)
{
	configurarProyeccion(ancho, alto);// Reconfigurar proyección al cambiar tamaño
}
// Procesar entrada del teclado
void procesarEntrada(GLFWwindow* ventana)
{
    // SALIR
    if (glfwGetKey(ventana, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(ventana, true);

    // --- CONTROL DE CÁMARA (FLECHAS) ---
    float velRotacion = 0.01f; // Velocidad de giro
    float velZoom = 0.01f;     // Velocidad de zoom

    // Rotación Y (Izquierda / Derecha)
	if (glfwGetKey(ventana, GLFW_KEY_RIGHT) == GLFW_PRESS) camYaw += velRotacion; // Girar a la derecha
	if (glfwGetKey(ventana, GLFW_KEY_LEFT) == GLFW_PRESS) camYaw -= velRotacion; // Girar a la izquierda

    // Rotación X (Arriba / Abajo)
	if (glfwGetKey(ventana, GLFW_KEY_UP) == GLFW_PRESS) camPitch -= velRotacion; // Girar hacia arriba
	if (glfwGetKey(ventana, GLFW_KEY_DOWN) == GLFW_PRESS) camPitch += velRotacion; // Girar hacia abajo

    // Limitar ángulo vertical para no dar la vuelta completa
	if (camPitch > 89.0f) camPitch = 89.0f;// Limitar ángulo máximo
	if (camPitch < -89.0f) camPitch = -89.0f;// Limitar ángulo mínimo

    // ZOOM (Teclas W y S)
	if (glfwGetKey(ventana, GLFW_KEY_W) == GLFW_PRESS) camDist -= velZoom; // Acercar
	if (glfwGetKey(ventana, GLFW_KEY_S) == GLFW_PRESS) camDist += velZoom;// Alejar

    // Limitar Zoom
	if (camDist < 5.0f)  camDist = 5.0f;// Distancia mínima
	if (camDist > 25.0f) camDist = 25.0f;// Distancia máxima


    // --- CONTROL DE MODELO (TRANSPARENCIA) ---
    // TECLA 1: Transparencia Izquierda
    if (glfwGetKey(ventana, GLFW_KEY_1) == GLFW_PRESS) {
        if (!tecla1Presionada) {
            transPulmonIzq = !transPulmonIzq;
            tecla1Presionada = true;
        }
    }
    else tecla1Presionada = false;

    // TECLA 2: Transparencia Derecha
    if (glfwGetKey(ventana, GLFW_KEY_2) == GLFW_PRESS) {
        if (!tecla2Presionada) {
            transPulmonDer = !transPulmonDer;
            tecla2Presionada = true;
        }
    }
    else tecla2Presionada = false;

    // TECLA 3: Mostrar/Ocultar Costillas
    if (glfwGetKey(ventana, GLFW_KEY_3) == GLFW_PRESS) {
        if (!tecla3Presionada) {
            mostrarCostillas = !mostrarCostillas;
            tecla3Presionada = true;
        }
    }
    else tecla3Presionada = false;
}

// --- GEOMETRÍA ---
void dibujarCilindroConAros(float radioBase, float alto, int slices, int stacks, int numAros, float amplitud)
{
	// Ajustar para cilindro invertido
	float longitud = std::fabs(alto);// Longitud del cilindro
	float signo = (alto >= 0.0f) ? 1.0f : -1.0f;// Signo para dirección
	// Dibujar cilindro con anillos
    for (int i = 0; i < stacks; ++i)
    {
        
		float t0 = (float)i / stacks; float t1 = (float)(i + 1) / stacks;// Parámetros de interpolación
		float y0 = signo * t0 * longitud; float y1 = signo * t1 * longitud;// Coordenadas Y
		float fase0 = t0 * (float)numAros * 6.2831f; float fase1 = t1 * (float)numAros * 6.2831f;// Fases para oscilación
		float r0 = radioBase * (1.0f + amplitud * sinf(fase0));// Radio con oscilación
		float r1 = radioBase * (1.0f + amplitud * sinf(fase1));// Radio con oscilación
		// Dibujar segmento del cilindro
        glBegin(GL_TRIANGLE_STRIP);
        for (int j = 0; j <= slices; ++j) {
            float s = (float)j / slices; float ang = s * 6.2831f;
            float cx = cosf(ang); float cz = sinf(ang);
            glNormal3f(cx, 0.0f, cz); glVertex3f(r0 * cx, y0, r0 * cz);
            glNormal3f(cx, 0.0f, cz); glVertex3f(r1 * cx, y1, r1 * cz);
        }
        glEnd();
    }
}
// Dibujar esfera para unión
void dibujarEsferaUnion(float radio)
{
    int lats = 20; int longs = 20;
    for (int i = 0; i <= lats; i++) {
        float lat0 = 3.14159f * (-0.5f + (float)(i - 1) / lats);
        float z0 = sin(lat0); float zr0 = cos(lat0);
        float lat1 = 3.14159f * (-0.5f + (float)i / lats);
        float z1 = sin(lat1); float zr1 = cos(lat1);

        glBegin(GL_QUAD_STRIP);
        for (int j = 0; j <= longs; j++) {
            float lng = 2 * 3.14159f * (float)(j - 1) / longs;
            float x = cos(lng); float y = sin(lng);
            glNormal3f(x * zr0, y * zr0, z0); glVertex3f(radio * x * zr0, radio * y * zr0, radio * z0);
            glNormal3f(x * zr1, y * zr1, z1); glVertex3f(radio * x * zr1, radio * y * zr1, radio * z1);
        }
        glEnd();
    }
}

void dibujarRama(float largo, float radio, int nivel)
{
    if (nivel == 0) return;
    int lados = 8 + nivel * 2;
    float pasoAngulo = 6.2831f / lados;

    glBegin(GL_TRIANGLE_STRIP);
    for (int i = 0; i <= lados; i++) {
        float ang = i * pasoAngulo;
        float x = cos(ang); float z = sin(ang);
        glNormal3f(x, 0.0f, z);
        glVertex3f(x * radio, 0.0f, z * radio);
        glVertex3f(x * radio * 0.7f, -largo, z * radio * 0.7f);
    }
    glEnd();
    glTranslatef(0.0f, -largo, 0.0f);

    float escalaLargo = 0.75f; float escalaRadio = 0.65f;
    glPushMatrix();
    glRotatef(35.0f + (nivel * 5.0f), 0.0f, 0.0f, 1.0f);
    glRotatef(20.0f, 0.0f, 1.0f, 0.0f);
    dibujarRama(largo * escalaLargo, radio * escalaRadio, nivel - 1);
    glPopMatrix();

    glPushMatrix();
    glRotatef(-30.0f - (nivel * 5.0f), 0.0f, 0.0f, 1.0f);
    glRotatef(-15.0f, 1.0f, 0.0f, 0.0f);
    dibujarRama(largo * escalaLargo, radio * escalaRadio, nivel - 1);
    glPopMatrix();
}

void dibujarTraqueaYBronquios()
{
    glColor4f(0.95f, 0.85f, 0.85f, 1.0f);

    // Traquea
    glPushMatrix();
    glTranslatef(0.0f, 1.8f, 0.0f);
    dibujarCilindroConAros(0.26f, -2.3f, 20, 32, 12, 0.12f);
    glPopMatrix();

    float yBifurcacion = -0.4f;
    glPushMatrix();
    glTranslatef(0.0f, yBifurcacion, 0.0f);
    glScalef(1.0f, 0.8f, 1.0f);
    dibujarEsferaUnion(0.28f);
    glPopMatrix();

    float yInicioBronquios = yBifurcacion + 0.15f;

    // Izquierdo
    glPushMatrix();
    glTranslatef(0.0f, yInicioBronquios, 0.0f);
    glRotatef(-40.0f, 0.0f, 0.0f, 1.0f);
    dibujarCilindroConAros(0.16f, -1.8f, 18, 20, 8, 0.10f);
    glPushMatrix();
    glTranslatef(0.0f, -1.8f, 0.0f);
    dibujarRama(0.8f, 0.12f, 3);
    glPopMatrix();
    glPopMatrix();

    // Derecho
    glPushMatrix();
    glTranslatef(0.0f, yInicioBronquios, 0.0f);
    glRotatef(40.0f, 0.0f, 0.0f, 1.0f);
    dibujarCilindroConAros(0.16f, -1.8f, 18, 20, 8, 0.10f);
    glPushMatrix();
    glTranslatef(0.0f, -1.8f, 0.0f);
    dibujarRama(0.8f, 0.12f, 3);
    glPopMatrix();
    glPopMatrix();
}

float perfilRadioPulmon(float t)
{
    if (t > 0.98f) return 0.0f;
    float curva = std::pow(std::sin(t * 3.14159f), 0.5f);
    float estrechamiento = 1.0f - (t * 0.55f);
    float radioFinal = 1.1f * curva * estrechamiento;
    if (t < 0.1f) radioFinal *= (t * 10.0f);
    return radioFinal;
}

void dibujarPulmonMalla(bool izquierdo)
{
    int stacks = 40; int slices = 40;
    float yMin = -1.6f; float yMax = 1.6f;
    float h = yMax - yMin;

    float escalaX = 1.20f * factorRespiracion;
    float escalaZ = 1.25f * factorRespiracion;

    for (int i = 0; i < stacks; ++i)
    {
        float t0 = (float)i / stacks; float t1 = (float)(i + 1) / stacks;
        float y0 = yMin + t0 * h; float y1 = yMin + t1 * h;
        float r0Base = perfilRadioPulmon(t0); float r1Base = perfilRadioPulmon(t1);

        float fuerza0 = (1.0f - t0) * (1.0f - t0);
        float fuerza1 = (1.0f - t1) * (1.0f - t1);
        float dir = izquierdo ? -1.0f : 1.0f;
        float off0 = fuerza0 * dir * 0.6f * factorRespiracion;
        float off1 = fuerza1 * dir * 0.6f * factorRespiracion;

        glBegin(GL_TRIANGLE_STRIP);
        for (int j = 0; j <= slices; ++j) {
            float s = (float)j / slices; float ang = s * 6.2831f;
            float c = cosf(ang); float si = sinf(ang);

            float n0 = 1.0f; float n1 = 1.0f;
            if (izquierdo && c < -0.15f) { float k = (c + 0.15f) / -0.85f; n0 = n1 = 1.0f - 0.35f * k; }
            else if (!izquierdo && c > 0.25f) { float k = (c - 0.25f) / 0.75f; n0 = n1 = 1.0f - 0.22f * k; }

            float r0 = r0Base * n0; float r1 = r1Base * n1;
            float x0 = (escalaX * r0 * c) + off0; float z0 = escalaZ * r0 * si;
            float x1 = (escalaX * r1 * c) + off1; float z1 = escalaZ * r1 * si;

            float nx0 = 0, ny0 = 1, nz0 = 0, nx1 = 0, ny1 = 1, nz1 = 0;
            if (t0 <= 0.95f) { float l = sqrt(x0 * x0 + y0 * y0 * 0.3f + z0 * z0); if (l < 0.001f)l = 1; nx0 = x0 / l; ny0 = y0 * 0.6f / l; nz0 = z0 / l; }
            if (t1 <= 0.95f) { float l = sqrt(x1 * x1 + y1 * y1 * 0.3f + z1 * z1); if (l < 0.001f)l = 1; nx1 = x1 / l; ny1 = y1 * 0.6f / l; nz1 = z1 / l; }

            glNormal3f(nx0, ny0, nz0); glTexCoord2f(s, t0); glVertex3f(x0, y0, z0);
            glNormal3f(nx1, ny1, nz1); glTexCoord2f(s, t1); glVertex3f(x1, y1, z1);
        }
        glEnd();
    }
}

void dibujarCostillas()
{
    glColor4f(0.9f, 0.9f, 0.85f, 1.0f); // Color Hueso

    // 1. COLUMNA VERTEBRAL (Atrás)
    glPushMatrix();
    glTranslatef(0.0f, -0.8f, 2.0f); // Posición trasera central
    glScalef(0.4f, 5.0f, 0.4f);      // Alta y delgada

    // Dibujar cilindro simple para la columna
    int ladosCol = 12;
    glBegin(GL_TRIANGLE_STRIP);
    for (int i = 0; i <= ladosCol; i++) {
        float a = i * 6.2831f / ladosCol;
        float x = cos(a); float z = sin(a);
        glNormal3f(x, 0, z);
        glVertex3f(x, 0.5f, z);
        glVertex3f(x, -0.6f, z);
    }
    glEnd();
    glPopMatrix();

    // 2. COSTILLAS (Arcos)
    int numCostillas = 10;
    for (int i = 0; i < numCostillas; i++)
    {
        float yPos = 1.0f - (i * 0.5f); // Altura de cada costilla

        // Ajuste de forma: Más anchas en el medio, más estrechas arriba/abajo
        // Ancho base 2.8 para librar los pulmones (que están en 1.6)
        float ancho = 2.8f + sin(i * 0.3f) * 0.4f;
        float profundidad = 1.8f;

        glPushMatrix();
        glTranslatef(0.0f, yPos, 0.0f); // Centradas en la columna
        glScalef(ancho, 0.15f, profundidad); // Achatadas (forma de cinta)

        // Dibujar el arco (Dejamos un hueco en frente para el esternón)
        glBegin(GL_TRIANGLE_STRIP);
        for (int k = -24; k <= 24; k++) // Rango de ángulo (no cierra el círculo completo)
        {
            float ang = k * 0.1f; // Radianes
            // Normales hacia afuera
            float x = sin(ang);
            float z = cos(ang); // Z positivo es hacia el frente aquí

            glNormal3f(x, 0, z);

            // Pared externa e interna de la costilla
            // Usamos un radio de 1.0 (base) y un grosor pequeño
            glVertex3f(x * 1.05f, 0.5f, z * 1.05f);
            glVertex3f(x * 0.95f, -0.5f, z * 0.95f);
        }
        glEnd();
        glPopMatrix();
    }
}

void dibujarModeloCompleto()
{
    // 1. DIBUJAR ESTRUCTURA INTERNA
    dibujarTraqueaYBronquios();

    // 2. Costillas (NUEVO)
    if (mostrarCostillas) {
        dibujarCostillas();
    }

    if (texturaPulmonID != 0) {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, texturaPulmonID);
    }

    float escalaGlobal = 1.6f;

    // --- PULMON IZQUIERDO ---
    glPushMatrix();
    glTranslatef(1.8f, -1.5f, 0.0f);
    glRotatef(0.0f, 0.0f, 0.0f, 1.0f);
    glScalef(escalaGlobal, escalaGlobal, escalaGlobal);

    if (transPulmonIzq) {
        glColor4f(1.0f, 1.0f, 1.0f, 0.4f);
    }
    else {
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    }

    dibujarPulmonMalla(true);
    glPopMatrix();

    // --- PULMON DERECHO ---
    glPushMatrix();
    glTranslatef(-1.8f, -1.5f, 0.0f);
    glRotatef(0.0f, 0.0f, 0.0f, 1.0f);
    glScalef(escalaGlobal, escalaGlobal, escalaGlobal);

    if (transPulmonDer) {
        glColor4f(1.0f, 1.0f, 1.0f, 0.4f);
    }
    else {
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    }

    dibujarPulmonMalla(false);
    glPopMatrix();

    if (texturaPulmonID != 0) glDisable(GL_TEXTURE_2D);
}

// --- MAIN ---
int main(void)
{
    GLFWwindow* window;
    if (!glfwInit()) return -1;

    window = glfwCreateWindow(ANCHO_VENTANA, ALTO_VENTANA, "Torax 3D - Flechas para mover, 1/2 Transparencia", NULL, NULL);
    if (!window) { glfwTerminate(); return -1; }

    glfwMakeContextCurrent(window);

    if (!inicializarOpenGL()) { glfwTerminate(); return -1; }

    glfwSetFramebufferSizeCallback(window, callbackCambioTamano);

    configurarProyeccion(ANCHO_VENTANA, ALTO_VENTANA);

    while (!glfwWindowShouldClose(window))
    {
        procesarEntrada(window);

        double tiempo = glfwGetTime();
        factorRespiracion = 1.0f + 0.05f * sin(tiempo * 2.0f);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glTranslatef(0.0f, 0.0f, -camDist);
        glRotatef(camPitch, 1.0f, 0.0f, 0.0f);
        glRotatef(camYaw, 0.0f, 1.0f, 0.0f);

        dibujarModeloCompleto();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}