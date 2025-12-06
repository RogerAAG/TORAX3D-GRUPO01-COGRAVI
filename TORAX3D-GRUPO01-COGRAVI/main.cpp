//-------------------------------------------------------------------------------------------------------------------------------//
// FACULTAD DE INGENIERÍA
// CARRERA DE INGENIERÍA DE SISTEMAS COMPUTACIONALES
// PROYECTO FINAL
// --- VISUALIZADOR 3D INTERACTIVO DEL TÓRAX HUMANO ---

// Autores:
// ALDANA GARCIA, ROGER ALEXANDER
// CHAVEZ AMBROSIO, JORGE JUNIOR
// LAZARO VARGAS, STEVEN ISAAC
// MOSCOSO TEMOCHE, DIANA TANEISHA
// TUESTA HUAMAN, CHRISTIAN MARTIN

// Curso:
// COMPUTACIÓN GRÁFICA Y VISUAL

// Docente del Curso :
// ROMERO UNTIVEROS, LUIS ALFREDO
//-------------------------------------------------------------------------------------------------------------------------------//
#include <iostream> //Se incluye la librería para manejo de entrada y salida
#include <vector> //Se incluye la librería para manejo de vectores
#include <cmath> //Se incluye la librería para funciones matemáticas
#include <GLFW/glfw3.h> //Se incluye la librería para manejo de ventanas y OpenGL
#define STB_IMAGE_IMPLEMENTATION //Definición para la implementación de la librería stb_image
#include "include/stb_image.h" //Se incluye la librería para carga de imágenes
#include "include/stb_easy_font.h"

 // --- CONSTANTES Y GLOBALES ---
// Dimensiones de la Ventana
const unsigned int ANCHO_VENTANA = 800; // Ancho de la ventana
const unsigned int ALTO_VENTANA = 600; // Alto de la ventana
const float PI = 3.14159265f; // Constante matemática para cálculos
// Variables de Cámara (Controlada por Teclado)
float camYaw = 45.0f; // Ángulo de rotación horizontal
float camPitch = 40.0f; // Ángulo de rotación vertical
float camDist = 15.0f; // Distancia de la cámara al origen
// Variables de Estado y Animación
float factorRespiracion = 1.0f; // Factor de escala para simulación de respiración
bool transPulmonIzq = false; // Transparencia del pulmón izquierdo
bool transPulmonDer = false; // Transparencia del pulmón derecho
bool mostrarCostillas = true; // Mostrar/Ocultar esqueleto
bool torsoTranslucido = false;	// Transparencia del torso

// Control de rebote de teclas (Toggle)
bool tecla1Presionada = false; // Estado de la tecla 1
bool tecla2Presionada = false; // Estado de la tecla 2
bool tecla3Presionada = false; // Estado de la tecla 3
bool tecla4Presionada = false; // Estado de la tecla 4
// Texturas
GLuint texturaPulmonID = 0; // ID de textura para los pulmones

// --- GESTIÓN DE RECURSOS (TEXTURAS) ---
// Función para cargar una textura desde archivo
bool cargarTextura(const char* ruta)
{
	int ancho, alto, canales; // Variables para dimensiones y canales
    // Cargar imagen usando librería externa
	unsigned char* data = stbi_load(ruta, &ancho, &alto, &canales, 0); // Cargar imagen
	// Verificar carga correcta
    if (!data)
    {
		std::cerr << "ERROR::TEXTURA::No se pudo cargar: " << ruta << "\n"; // Mensaje de error
        return false;
    }

    // Generar y configurar textura OpenGL
	glGenTextures(1, &texturaPulmonID);// Generar ID de textura
	glBindTexture(GL_TEXTURE_2D, texturaPulmonID);// Enlazar textura
    // Configurar filtrado y repetición
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);// Filtrado lineal
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);// Filtrado lineal
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);// Repetición en S
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);// Repetición en T
    // Cargar datos a la GPU
	GLenum formato = (canales == 4) ? GL_RGBA : GL_RGB; // Determinar formato
	glTexImage2D(GL_TEXTURE_2D, 0, formato, ancho, alto, 0, formato, GL_UNSIGNED_BYTE, data); // Cargar datos
	// Liberar datos de la imagen
    stbi_image_free(data); // Liberar memoria RAM
    return true;
}

// --- CONFIGURACIÓN INICIAL DE OPENGL ---
// Función para inicializar OpenGL y cargar recursos
bool inicializarOpenGL()
{
    /* Configuración de capacidades */
    glEnable(GL_DEPTH_TEST);     // Z-Buffer para profundidad correcta
    glEnable(GL_LIGHTING);       // Sistema de iluminación
    glEnable(GL_LIGHT0);         // Luz principal
    glEnable(GL_COLOR_MATERIAL); // Materiales responden a glColor
    glEnable(GL_NORMALIZE);      // Normalizar vectores para luces correctas
    glEnable(GL_BLEND);          // Transparencia
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // Función de mezcla alfa

    glClearColor(0.05f, 0.05f, 0.08f, 1.0f); // Fondo Gris Oscuro

    /* Configuración de Iluminación */
	GLfloat posicionLuz[] = { 4.0f, 6.0f, 5.0f, 1.0f }; // Posición de la luz
	glLightfv(GL_LIGHT0, GL_POSITION, posicionLuz); // Posición de la luz

	GLfloat luzAmbiente[] = { 0.3f, 0.3f, 0.3f, 1.0f }; // Luz ambiental
	GLfloat luzDifusa[] = { 0.9f, 0.9f, 0.9f, 1.0f }; // Luz difusa
	GLfloat luzEspecular[] = { 0.3f, 0.3f, 0.3f, 1.0f }; // Luz especular

	glLightfv(GL_LIGHT0, GL_AMBIENT, luzAmbiente); // Configuración de luz ambiental
	glLightfv(GL_LIGHT0, GL_DIFFUSE, luzDifusa); // Configuración de luz difusa
	glLightfv(GL_LIGHT0, GL_SPECULAR, luzEspecular); // Configuración de luz especular

    /* Configuración de Materiales */
	GLfloat matEspecular[] = { 0.4f, 0.4f, 0.4f, 1.0f }; // Material especular
	GLfloat shininess[] = { 32.0f }; // Brillo del material
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, matEspecular); // Configuración de material especular
	glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, shininess); // Configuración de brillo
	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE); // Material responde a color ambiente y difuso

    /* Carga de Recursos */
    stbi_set_flip_vertically_on_load(true);
	// Cargar textura de los pulmones
    if (!cargarTextura("textures/pulmon1.jpg"))
    {
        return false;
    }
	// Éxito
    return true;
}

// --- SISTEMA DE CÁMARA Y PROYECCIÓN ---
// Función para configurar la proyección y viewport
void configurarProyeccion(int ancho, int alto)
{
    if (alto == 0) alto = 1; // Prevenir división por cero
    
	glViewport(0, 0, ancho, alto);// Configurar viewport
	glMatrixMode(GL_PROJECTION);// Seleccionar matriz de proyección
	glLoadIdentity();// Resetear matriz

    // Configuración manual del Frustum (Perspectiva)
	float aspect = (float)ancho / (float)alto; // Relación de aspecto
	float fovY = 45.0f; // Campo de visión vertical
	float nearPlane = 0.1f; // Plano cercano
	float farPlane = 100.0f; // Plano lejano
	// Cálculo de los límites del frustum
	float top = nearPlane * tanf(fovY * PI / 360.0f); // Mitad altura en el plano cercano
	float bottom = -top; // Mitad inferior
	float right = top * aspect; // Mitad derecha
	float left = -right; // Mitad izquierda
	// Configurar frustum
    glFrustum(left, right, bottom, top, nearPlane, farPlane);

	glMatrixMode(GL_MODELVIEW); // Volver a modelo-vista
	glLoadIdentity(); // Resetear matriz
}

// Función para posicionar la cámara
void callbackCambioTamano(GLFWwindow* ventana, int ancho, int alto)
{
	configurarProyeccion(ancho, alto); // Reconfigurar proyección
}

// --- PROCESAMIENTO DE ENTRADA (INPUT) ---
// Función para procesar la entrada del usuario
void procesarEntrada(GLFWwindow* ventana)
{
    // Salir de la aplicación
    if (glfwGetKey(ventana, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(ventana, true);

    /* Control de Cámara (Flechas y W/S) */
    float velRotacion = 0.1f; // Velocidad ajustada
	float velZoom = 0.03f; // Velocidad de zoom

    // Rotación
	if (glfwGetKey(ventana, GLFW_KEY_RIGHT) == GLFW_PRESS) camYaw += velRotacion; // Rotar derecha
	if (glfwGetKey(ventana, GLFW_KEY_LEFT) == GLFW_PRESS)  camYaw -= velRotacion; // Rotar izquierda
	if (glfwGetKey(ventana, GLFW_KEY_UP) == GLFW_PRESS)    camPitch -= velRotacion; // Rotar arriba
	if (glfwGetKey(ventana, GLFW_KEY_DOWN) == GLFW_PRESS)  camPitch += velRotacion; // Rotar abajo

    // Limitar rotación vertical
	if (camPitch > 89.0f) camPitch = 89.0f; // Limite superior
	if (camPitch < -89.0f) camPitch = -89.0f; // Limite inferior

    // Zoom
	if (glfwGetKey(ventana, GLFW_KEY_W) == GLFW_PRESS) camDist -= velZoom; // Acercar
	if (glfwGetKey(ventana, GLFW_KEY_S) == GLFW_PRESS) camDist += velZoom; // Alejar

    // Limitar Zoom
	if (camDist < 5.0f)  camDist = 5.0f; // Limite cercano
	if (camDist > 25.0f) camDist = 25.0f;  // Limite lejano

    /* Control de Interactividad (Teclas 1, 2, 3) */
    // Tecla 1: Transparencia Izquierda
    if (glfwGetKey(ventana, GLFW_KEY_1) == GLFW_PRESS) 
    {
		// Verificar rebote de tecla
        if (!tecla1Presionada) 
        {
			transPulmonIzq = !transPulmonIzq;// Alternar transparencia
			tecla1Presionada = true;// Marcar como presionada
        }
    }
	else tecla1Presionada = false; // Resetear estado de tecla

    // Tecla 2: Transparencia Derecha
    if (glfwGetKey(ventana, GLFW_KEY_2) == GLFW_PRESS) 
    {
        if (!tecla2Presionada) 
        {
			transPulmonDer = !transPulmonDer; // Alternar transparencia
			tecla2Presionada = true; // Marcar como presionada
        }
    }
	else tecla2Presionada = false; // Resetear estado de tecla

    // Tecla 3: Mostrar/Ocultar Esqueleto
    if (glfwGetKey(ventana, GLFW_KEY_3) == GLFW_PRESS) 
    {
        if (!tecla3Presionada) 
        {
			mostrarCostillas = !mostrarCostillas; // Alternar visibilidad
			tecla3Presionada = true; // Marcar como presionada
        }
    }
	else tecla3Presionada = false; // Resetear estado de tecla
	// Tecla 4: Modo torso translúcido
	if (glfwGetKey(ventana, GLFW_KEY_4) == GLFW_PRESS) 
	{
		if (!tecla4Presionada) // Verificar rebote de tecla
		{
			torsoTranslucido = !torsoTranslucido; // Alternar modo
			tecla4Presionada = true; // Marcar como presionada
		}
	}
	else tecla4Presionada = false; // Resetear estado de tecla
}

// --- FUNCIONES DE GEOMETRÍA (MODELADO) ---
// Primitiva: Cilindro con deformación sinusoidal (Anillos) para representar la traquea.
void dibujarCilindroConAros(float radioBase, float alto, int slices, int stacks, int numAros, float amplitud)
{
	// Ajustar longitud y dirección
    float longitud = std::fabs(alto);
    float signo = (alto >= 0.0f) ? 1.0f : -1.0f;
	// Construcción del cilindro con anillos
    for (int i = 0; i < stacks; ++i)
    {
		float t0 = (float)i / stacks; // Parámetro de altura
		float t1 = (float)(i + 1) / stacks; // Parámetro de altura siguiente
		float y0 = signo * t0 * longitud; // Altura actual
		float y1 = signo * t1 * longitud; // Altura siguiente

        // Cálculo de ondas para los anillos cartilaginosos
		float fase0 = t0 * (float)numAros * 2.0f * PI; // Fase de la onda en y0
		float fase1 = t1 * (float)numAros * 2.0f * PI; // Fase de la onda en y1
		float r0 = radioBase * (1.0f + amplitud * sinf(fase0)); // Radio deformado en y0
		float r1 = radioBase * (1.0f + amplitud * sinf(fase1)); // Radio deformado en y1
		// Dibujar segmento del cilindro
        glBegin(GL_TRIANGLE_STRIP);
		// Iterar sobre los slices
        for (int j = 0; j <= slices; ++j) 
        {
			float s = (float)j / slices;    // Parámetro angular
			float ang = s * 2.0f * PI;  // Ángulo actual
			float cx = cosf(ang);   // Componente X
			float cz = sinf(ang);   // Componente Z

			glNormal3f(cx, 0.0f, cz);   // Normal en y0
			glVertex3f(r0 * cx, y0, r0 * cz);   // Vértice en y0

			glNormal3f(cx, 0.0f, cz);   // Normal en y1
			glVertex3f(r1 * cx, y1, r1 * cz);   // Vértice en y1
        }
		glEnd(); // Finalizar segmento
    }
}

// Primitiva: Esfera (Usada para la Carina)
void dibujarEsferaUnion(float radio)
{
	// Construcción de la esfera usando quad strips
    int lats = 20; int longs = 20;
	// Iterar sobre latitudes
    for (int i = 0; i <= lats; i++) 
    {
		float lat0 = PI * (-0.5f + (float)(i - 1) / lats);  // Latitud inicial
		float z0 = sin(lat0); float zr0 = cos(lat0);    // Coordenadas Z y radio en latitud
		float lat1 = PI * (-0.5f + (float)i / lats);    // Latitud siguiente
		float z1 = sin(lat1); float zr1 = cos(lat1);    // Coordenadas Z y radio en latitud siguiente
		// Iterar sobre longitudes
        glBegin(GL_QUAD_STRIP);
        for (int j = 0; j <= longs; j++) 
        {
			float lng = 2 * PI * (float)(j - 1) / longs; // Longitud actual
			float x = cos(lng); float y = sin(lng); // Coordenadas X e Y
			// Vértices y normales
			glNormal3f(x * zr0, y * zr0, z0); // Normal en lat0
			glVertex3f(radio * x * zr0, radio * y * zr0, radio * z0); // Vértice en lat0
			glNormal3f(x * zr1, y * zr1, z1); // Normal en lat1
			glVertex3f(radio * x * zr1, radio * y * zr1, radio * z1); // Vértice en lat1
        }
		glEnd(); // Finalizar quad strip
    }
}

// Primitiva: Rama Fractal (Bronquiolos)
void dibujarRama(float largo, float radio, int nivel)
{
	if (nivel == 0) return; // Caso base de la recursión
	int lados = 8 + nivel * 2; // Más lados para niveles superiores
	float pasoAngulo = 2.0f * PI / lados; // Paso angular
	// Cuerpo de la rama
    glBegin(GL_TRIANGLE_STRIP);
	// Dibujar cilindro
    for (int i = 0; i <= lados; i++) 
    {
		float ang = i * pasoAngulo; // Ángulo actual
		float x = cos(ang); float z = sin(ang); // Coordenadas X y Z
		glNormal3f(x, 0.0f, z); // Normal
		glVertex3f(x * radio, 0.0f, z * radio); // Vértice inferior
		glVertex3f(x * radio * 0.7f, -largo, z * radio * 0.7f); // Vértice superior
    }
	glEnd(); // Finalizar cilindro
	// Mover al extremo superior de la rama
    glTranslatef(0.0f, -largo, 0.0f);
	// Escalas para sub-ramas
	float escalaLargo = 0.75f; // Escala de largo
	float escalaRadio = 0.65f; // Escala de radio

    // Sub-rama Izquierda
	glPushMatrix(); // Guardar estado
	glRotatef(35.0f + (nivel * 5.0f), 0.0f, 0.0f, 1.0f); // Rotar para izquierda
	glRotatef(20.0f, 0.0f, 1.0f, 0.0f); // Rotar hacia afuera
	dibujarRama(largo * escalaLargo, radio * escalaRadio, nivel - 1); // Llamada recursiva
	glPopMatrix(); // Restaurar estado

    // Sub-rama Derecha
	glPushMatrix(); // Guardar estado
	glRotatef(-30.0f - (nivel * 5.0f), 0.0f, 0.0f, 1.0f); // Rotar para derecha
	glRotatef(-15.0f, 1.0f, 0.0f, 0.0f); // Rotar hacia afuera
	dibujarRama(largo * escalaLargo, radio * escalaRadio, nivel - 1); // Llamada recursiva
	glPopMatrix(); // Restaurar estado
}

// Modelo: Sistema Traqueal
void dibujarTraqueaYBronquios()
{
    glColor4f(0.95f, 0.85f, 0.85f, 1.0f); // Color hueso/cartílago
    // Traquea Principal
	glPushMatrix(); // Guardar estado
	glTranslatef(0.0f, 1.8f, 0.0f); // Posicionar al inicio de la traquea
	dibujarCilindroConAros(0.26f, -2.3f, 20, 32, 12, 0.12f); // Dibujar traquea
	glPopMatrix(); // Restaurar estado

    // Carina (Unión)
	float yBifurcacion = -0.4f; // Altura de la bifurcación
	glPushMatrix(); // Guardar estado
	glTranslatef(0.0f, yBifurcacion, 0.0f); // Posicionar en la bifurcación
	glScalef(1.0f, 0.8f, 1.0f); // Escalar ligeramente en Y
	dibujarEsferaUnion(0.28f); // Dibujar esfera de unión
	glPopMatrix(); // Restaurar estado
	// Inicio de los Bronquios
	float yInicioBronquios = yBifurcacion + 0.15f; // Altura de inicio de bronquios

    // Bronquio y Árbol Izquierdo
	glPushMatrix(); // Guardar estado
	glTranslatef(0.0f, yInicioBronquios, 0.0f); // Posicionar al inicio del bronquio
	glRotatef(-40.0f, 0.0f, 0.0f, 1.0f); // Rotar hacia la izquierda
	dibujarCilindroConAros(0.16f, -1.8f, 18, 20, 8, 0.10f); // Dibujar bronquio
	// Árbol de Bronquiolos Izquierdo
	glPushMatrix(); // Guardar estado
	glTranslatef(0.0f, -1.8f, 0.0f); // Posicionar al final del bronquio
	dibujarRama(0.8f, 0.12f, 3); // Dibujar rama fractal
	glPopMatrix(); // Restaurar estado izquierdo
	glPopMatrix(); // Restaurar estado derecho

    // Bronquio y Árbol Derecho
	glPushMatrix(); // Guardar estado
	glTranslatef(0.0f, yInicioBronquios, 0.0f); // Posicionar al inicio del bronquio
	glRotatef(40.0f, 0.0f, 0.0f, 1.0f); // Rotar hacia la derecha
	dibujarCilindroConAros(0.16f, -1.8f, 18, 20, 8, 0.10f); // Dibujar bronquio
	// Árbol de Bronquiolos Derecho
	glPushMatrix(); // Guardar estado
	glTranslatef(0.0f, -1.8f, 0.0f); // Posicionar al final del bronquio
	dibujarRama(0.8f, 0.12f, 3); // Dibujar rama fractal
	glPopMatrix(); // Restaurar estado derecho
	glPopMatrix(); // Restaurar estado izquierdo
}

// Matemática: Perfil de la forma del pulmón
float perfilRadioPulmon(float t)
{
    if (t > 0.98f) return 0.0f; // Cerrar punta superior
	float curva = std::pow(std::sin(t * PI), 0.5f); // Curva base
	float estrechamiento = 1.0f - (t * 0.55f); // Estrechamiento inferior
	float radioFinal = 1.1f * curva * estrechamiento; // Radio final
    if (t < 0.1f) radioFinal *= (t * 10.0f); // Suavizar base
	return radioFinal; // Retornar radio
}

// Modelo: Malla del Pulmón
void dibujarPulmonMalla(bool izquierdo)
{
	int stacks = 40; int slices = 40; // Resolución de la malla
	float yMin = -1.6f; float yMax = 1.6f; // Alturas
	float h = yMax - yMin; // Altura total

    // Animación de respiración
	float escalaX = 1.20f * factorRespiracion; // Escala en X
	float escalaZ = 1.25f * factorRespiracion; // Escala en Z
	// Construcción de la malla
    for (int i = 0; i < stacks; ++i)
    { 
		// Alturas y radios en los niveles actuales
		float t0 = (float)i / stacks; // Parámetro de altura
		float t1 = (float)(i + 1) / stacks; // Parámetro de altura siguiente
		float y0 = yMin + t0 * h; // Altura en t0
		float y1 = yMin + t1 * h; // Altura en t1
		float r0Base = perfilRadioPulmon(t0); // Radio base en t0
		float r1Base = perfilRadioPulmon(t1); // Radio base en t1

        // Deformación inferior (picos)
		float fuerza0 = (1.0f - t0) * (1.0f - t0); // Fuerza de deformación en t0
		float fuerza1 = (1.0f - t1) * (1.0f - t1); // Fuerza de deformación en t1
		float dir = izquierdo ? -1.0f : 1.0f; // Dirección según pulmón
		float off0 = fuerza0 * dir * 0.6f * factorRespiracion; // Offset en t0
		float off1 = fuerza1 * dir * 0.6f * factorRespiracion; // Offset en t1
		// Dibujar segmento de la malla
        glBegin(GL_TRIANGLE_STRIP);
        for (int j = 0; j <= slices; ++j)
        {
			// Cálculo de posición angular
			float s = (float)j / slices; // Parámetro angular
			float ang = s * 2.0f * PI; // Ángulo actual
			float c = cosf(ang); // Componente coseno
			float si = sinf(ang); // Componente seno

            // Incisura Cardiaca
			float n0 = 1.0f, n1 = 1.0f; // Factores de normalización
			if (izquierdo && c < -0.15f) // Ajuste para pulmón izquierdo
            {
				float k = (c + 0.15f) / -0.85f; n0 = n1 = 1.0f - 0.35f * k; // Ajuste más pronunciado
            }
            else if (!izquierdo && c > 0.25f) 
            {
				float k = (c - 0.25f) / 0.75f; n0 = n1 = 1.0f - 0.22f * k; // Ajuste menos pronunciado
            }
			// Cálculo de posiciones finales
			float r0 = r0Base * n0; // Radio ajustado en t0
			float r1 = r1Base * n1; // Radio ajustado en t1
			// Coordenadas X y Z
			float x0 = (escalaX * r0 * c) + off0; // X en t0
			float z0 = escalaZ * r0 * si; // Z en t0
			float x1 = (escalaX * r1 * c) + off1; // X en t1
			float z1 = escalaZ * r1 * si; // Z en t1

            // Cálculo de normales para iluminación
            float nx0 = 0, ny0 = 1, nz0 = 0, nx1 = 0, ny1 = 1, nz1 = 0;
            if (t0 <= 0.95f) 
            {
				float l = sqrt(x0 * x0 + y0 * y0 * 0.3f + z0 * z0); if (l < 0.001f)l = 1; // Evitar división por cero
				nx0 = x0 / l; ny0 = y0 * 0.6f / l; nz0 = z0 / l; // Normal en t0
            }
            if (t1 <= 0.95f) 
            {
				float l = sqrt(x1 * x1 + y1 * y1 * 0.3f + z1 * z1); if (l < 0.001f)l = 1; // Evitar división por cero
				nx1 = x1 / l; ny1 = y1 * 0.6f / l; nz1 = z1 / l; // Normal en t1
            }

			glNormal3f(nx0, ny0, nz0); glTexCoord2f(s, t0); glVertex3f(x0, y0, z0); // Vértice en t0
			glNormal3f(nx1, ny1, nz1); glTexCoord2f(s, t1); glVertex3f(x1, y1, z1); // Vértice en t1
        }
		glEnd(); // Finalizar segmento
    }
}

// Primitiva: Vértebra Individual
void dibujarVertebra(float radio, float altura)
{
	// Cuerpo Vertebral
    int lados = 12;
    // Cuerpo
    glBegin(GL_TRIANGLE_STRIP);
	// Dibujar cilindro
    for (int i = 0; i <= lados; i++) 
    {
		float a = i * 2.0f * PI / lados; // Ángulo actual
		float x = cos(a); float z = sin(a); // Coordenadas X y Z
		glNormal3f(x, 0, z); // Normal lateral
		glVertex3f(x * radio, altura / 2, z * radio); // Vértice superior
		glVertex3f(x * radio, -altura / 2, z * radio); // Vértice inferior
    }
	glEnd(); // Finalizar cuerpo

    // Apófisis Espinosa
	glPushMatrix(); // Guardar estado
	glTranslatef(0.0f, 0.0f, -radio); // Posicionar atrás
	glScalef(0.15f, altura * 0.8f, 0.5f); // Escalar apófisis
	glBegin(GL_QUADS); // Dibujar cubo
	glNormal3f(0, 0, -1); // Cara trasera
	glVertex3f(-0.5f, 0.5f, -1.0f); glVertex3f(0.5f, 0.5f, -1.0f); // Vértices
	glVertex3f(0.5f, -0.5f, -1.0f); glVertex3f(-0.5f, -0.5f, -1.0f); // Vértices
    // Lados...
	glNormal3f(1, 0, 0); glVertex3f(0.5f, 0.5f, 0.0f); glVertex3f(0.5f, 0.5f, -1.0f); // Vértices
	glVertex3f(0.5f, -0.5f, -1.0f); glVertex3f(0.5f, -0.5f, 0.0f); // Vértices
	glNormal3f(-1, 0, 0); glVertex3f(-0.5f, 0.5f, -1.0f); glVertex3f(-0.5f, 0.5f, 0.0f); // Vértices
	glVertex3f(-0.5f, -0.5f, 0.0f); glVertex3f(-0.5f, -0.5f, -1.0f); // Vértices
	glEnd(); // Finalizar apófisis
	glPopMatrix(); // Restaurar estado
}

// Modelo: Esqueleto (Columna y Costillas)
void dibujarCostillas()
{
	glColor4f(0.9f, 0.9f, 0.85f, 1.0f); // Color hueso

    // 1. COLUMNA VERTEBRAL
	glPushMatrix(); // Guardar estado
	glTranslatef(0.0f, -0.5f, -2.7f); // Posicionar columna
	int numVertebras = 18; // Número de vértebras
	float alturaTotal = 5.0f; // Altura total de la columna
	float alturaVertebra = alturaTotal / numVertebras; // Altura por vértebra
	// Dibujar cada vértebra
    for (int i = 0; i < numVertebras; i++)
    {
		glPushMatrix(); // Guardar estado
		float yRel = (i - numVertebras / 2.0f) * alturaVertebra; // Posición Y relativa
		glTranslatef(0.0f, yRel, 0.0f); // Posicionar vértebra
		float escalaAncho = 1.0f + 0.1f * sin(i * 0.5f); // Variación de ancho
		dibujarVertebra(0.35f * escalaAncho, alturaVertebra * 0.85f); // Dibujar vértebra
		glPopMatrix(); // Restaurar estado
    }
	glPopMatrix();	// Restaurar estado

    // 2. COSTILLAS
	int numCostillas = 10; // Número de costillas
	int ladosTubo = 12; // Lados del tubo de la costilla
	float radioTubo = 0.08f; // Radio del tubo de la costilla
	// Dibujar cada costilla
    for (int i = 0; i < numCostillas; i++)
    {
		float yPos = 1.2f - (i * 0.7f); // Posición Y de la costilla
		float ancho = 2.5f + sin(i * 0.35f) * 1.0f; // Ancho variable
		float profundidad = 2.5f; // Profundidad constante
        float anguloCaida = 10.0f + (i * i * 0.5f); // Curva exponencial
        int limiteArco = 29 - (i * 0.7f); // Apertura progresiva
		// Dibujar costilla como tubo curvado
		glPushMatrix(); // Guardar estado
		glTranslatef(0.0f, yPos, -0.3f); // Posicionar costilla
		glRotatef(anguloCaida, 1.0f, 0.0f, 0.0f); // Inclinación hacia adelante
		glScalef(ancho, 1.0f, profundidad); // Escalar costilla
		// Dibujar tubo curvado
        for (int k = -limiteArco; k < limiteArco; k++)
        {
			float ang1 = k * 0.1f; // Ángulo inicial
			float ang2 = (k + 1) * 0.1f; // Ángulo siguiente
			// Dibujar segmento del tubo
            glBegin(GL_TRIANGLE_STRIP);
			// Iterar sobre los lados del tubo
            for (int j = 0; j <= ladosTubo; j++)
            {
				float angTubo = j * 2.0f * PI / ladosTubo; // Ángulo alrededor del tubo
				float c_tubo = cos(angTubo); // Componente coseno 
				float s_tubo = sin(angTubo); // Componente seno
				// Vértices y normales
				float R1 = 1.0f + radioTubo * c_tubo; // Radio en ang1
				float x1 = sin(ang1) * R1; float y1 = radioTubo * s_tubo; float z1 = -cos(ang1) * R1; // Coordenadas en ang1
				float nx1 = sin(ang1) * c_tubo; float ny1 = s_tubo; float nz1 = -cos(ang1) * c_tubo; // Normal en ang1
				// Vértice en ang1
                glNormal3f(nx1, ny1, nz1); glVertex3f(x1, y1, z1); 
				// Vértice en ang2
				float R2 = 1.0f + radioTubo * c_tubo; // Radio en ang2
				float x2 = sin(ang2) * R2; float y2 = y1; float z2 = -cos(ang2) * R2; // Coordenadas en ang2
				float nx2 = sin(ang2) * c_tubo; float nz2 = -cos(ang2) * c_tubo; // Normal en ang2
				// Vértice en ang2
                glNormal3f(nx2, ny1, nz2); glVertex3f(x2, y2, z2);
            }
			glEnd(); // Finalizar segmento
        }
		glPopMatrix(); // Restaurar estado
    }
}

//Perfil del torso para modelado
float perfilRadioTorso(float t)
{
	// Perfil base sinusoidal
	float base = 0.7f + 0.7f * std::sin(t * PI);
	// Ajustes para cintura y pecho
	float cintura = std::exp(-((t - 0.3f) * (t - 0.3f)) / 0.02f); // Cintura
	base -= 0.35f * cintura; // Reducir en cintura
	// Realce del pecho
	float pecho = std::exp(-((t - 0.7f) * (t - 0.7f)) / 0.03f);// Pecho
	base += 0.45f * pecho; // Aumentar en pecho
	// Suavizado superior
	base *= (1.0f - 0.35f * t); // Suavizar hacia arriba
	// Asegurar no negativo
	if (base < 0.0f) base = 0.0f;
	return base;
}

//Modelo: Torso de Tejido Blando, cobertura externa
void dibujarTorsoTejidoBlando()
{
	glPushMatrix(); // Guardar estado
	// Posicionar torso
	glTranslatef(0.0f, -3.5f, -1.f); // Ajuste vertical y profundidad
	// Escalar torso
	float escalaTorso = 3.2f;  // Escala general del torso
	glScalef(escalaTorso, escalaTorso, escalaTorso); // Escala uniforme
	// Configurar color y transparencia
	float alphaTorso = torsoTranslucido ? 0.07f : 1.0f;  // Transparencia según modo
	glColor4f(1.0f, 0.8f, 0.7f, alphaTorso); // Color piel
	// Construcción del torso
	int stacks = 40; // Número de divisiones verticales
	int slices = 48; // Número de divisiones alrededor
	// Altura del torso
	float yMin = -2.4f; // Altura mínima
	float yMax = 2.6f; // Altura máxima
	float h = yMax - yMin; // Altura total

	// Dibujar malla del torso
	for (int i = 0; i < stacks; ++i)
	{
		float t0 = (float)i / stacks;// Parámetro de altura
		float t1 = (float)(i + 1) / stacks; // Parámetro de altura siguiente
		// Alturas en los niveles actuales
		float y0 = yMin + t0 * h; // Altura en t0
		float y1 = yMin + t1 * h; // Altura en t1
		// Radios base en los niveles actuales
		float r0Base = perfilRadioTorso(t0); // Radio base en t0
		float r1Base = perfilRadioTorso(t1); // Radio base en t1
		// Dibujar segmento de la malla
		glBegin(GL_TRIANGLE_STRIP);
		// Iterar sobre los slices
		for (int j = 0; j <= slices; ++j)
		{
			// Cálculo de posición angular
			float s = (float)j / slices; // Parámetro angular
			float ang = s * 2.0f * PI; // Ángulo actual
			// Componentes trigonométricas
			float c = cosf(ang); // Componente coseno
			float si = sinf(ang); // Componente seno

			// Elipse: más ancho en X que en Z (pecho más ancho que profundo)
			float rx0 = r0Base * 1.4f; // Radio en X en t0
			float rz0 = r0Base * 0.9f; // Radio en Z en t0
			float rx1 = r1Base * 1.4f; // Radio en X en t1
			float rz1 = r1Base * 0.9f; // Radio en Z en t1

			// Aplanar ligeramente el pecho (zona frontal, Z negativa)
			if (si < 0.0f)
			{
				float k = -si; // 0 en laterales, 1 en pura frente
				rx0 *= (0.9f + 0.1f * k); // Ajuste en t0
				rx1 *= (0.9f + 0.1f * k); // Ajuste en t1
				rz0 *= (0.7f + 0.3f * k); // Ajuste en t0
				rz1 *= (0.7f + 0.3f * k); // Ajuste en t1
			}
			// Coordenadas finales
			float x0 = rx0 * c; // X en t0
			float z0 = rz0 * si; // Z en t0
			float x1 = rx1 * c; // X en t1
			float z1 = rz1 * si; // Z en t1

			// Normales aproximadas para iluminación 
			float len0 = std::sqrt(x0 * x0 + (y0 * 0.4f) * (y0 * 0.4f) + z0 * z0); // Longitud del vector
			if (len0 < 1e-4f) len0 = 1.0f; // Evitar división por cero
			float len1 = std::sqrt(x1 * x1 + (y1 * 0.4f) * (y1 * 0.4f) + z1 * z1); // Longitud del vector
			if (len1 < 1e-4f) len1 = 1.0f; // Evitar división por cero
			// Vértices y normales
			glNormal3f(x0 / len0, (y0 * 0.4f) / len0, z0 / len0); // Normal en t0
			glVertex3f(x0, y0, z0); // Vértice en t0

			glNormal3f(x1 / len1, (y1 * 0.4f) / len1, z1 / len1); // Normal en t1
			glVertex3f(x1, y1, z1); // Vértice en t1
		}
		glEnd();// Finalizar segmento
	}

	glPopMatrix(); // Restaurar estado
}


// Modelo Completo: Combina todos los elementos
void dibujarModeloCompleto()
{
    // 1. Dibujar Estructura Interna
    dibujarTraqueaYBronquios();
    // 2. Dibujar Esqueleto (Si está activo)
    if (mostrarCostillas) 
	{
        dibujarCostillas();
    }
    // 3. Configurar Texturas para Pulmones
    if (texturaPulmonID != 0) 
	{
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, texturaPulmonID);
    }
	// 4. Dibujar Pulmones
    float escalaGlobal = 1.6f;

    // --- PULMON IZQUIERDO ---
	glPushMatrix(); // Guardar estado
	glTranslatef(1.8f, -1.5f, 0.0f); // Posicionar pulmón izquierdo
	glRotatef(0.0f, 0.0f, 0.0f, 1.0f); // Rotación (si es necesario)
	glScalef(escalaGlobal, escalaGlobal, escalaGlobal); // Escala global
	// Configurar color y transparencia
	if (transPulmonIzq) glColor4f(1.0f, 1.0f, 1.0f, 0.4f); // Transparente
	else glColor4f(1.0f, 1.0f, 1.0f, 1.0f); // Opaco
	// Dibujar malla del pulmón izquierdo
    dibujarPulmonMalla(true);
	// Restaurar estado
    glPopMatrix();

    // --- PULMON DERECHO ---
	glPushMatrix(); // Guardar estado
	glTranslatef(-1.8f, -1.5f, 0.0f); // Posicionar pulmón derecho
	glRotatef(0.0f, 0.0f, 0.0f, 1.0f); // Rotación (si es necesario)
	glScalef(escalaGlobal, escalaGlobal, escalaGlobal); // Escala global
	// Configurar color y transparencia
	if (transPulmonDer) glColor4f(1.0f, 1.0f, 1.0f, 0.4f); // Transparente
	else glColor4f(1.0f, 1.0f, 1.0f, 1.0f); // Opaco
	// Dibujar malla del pulmón derecho
    dibujarPulmonMalla(false);
	// Restaurar estado
    glPopMatrix();

	if (texturaPulmonID != 0) glDisable(GL_TEXTURE_2D);// Desactivar texturas

	// 5. Dibujar Tejido Blando / Torso (encima, semi-transparente)
	dibujarTorsoTejidoBlando(); // Dibujar torso
}

// Texto 2D en pantalla usando stb_easy_font
void dibujarTexto2D(const char* texto, float x, float y, float r, float g, float b)
{
	// Buffer para vértices
	char buffer[99999];
	// Generar geometría del texto
	int num_quads = stb_easy_font_print(
		0.0f, 0.0f,           
		(char*)texto,
		NULL,
		buffer, sizeof(buffer)
	);
	// Configurar estado OpenGL
	glDisable(GL_LIGHTING); // Desactivar iluminación
	glDisable(GL_TEXTURE_2D); // Desactivar texturas
	glColor3f(r, g, b);// Color del texto
	glPushMatrix(); // Guardar estado
	// Posicionar texto
	glTranslatef(x, y, 0.0f);
	// Escalar texto
	glScalef(2.0f, 2.0f, 1.0f);
	// Dibujar texto usando vertex arrays
	glEnableClientState(GL_VERTEX_ARRAY); // Habilitar array de vértices
	glVertexPointer(2, GL_FLOAT, 16, buffer); // Configurar puntero de vértices
	glDrawArrays(GL_QUADS, 0, num_quads * 4); // Dibujar quads
	glDisableClientState(GL_VERTEX_ARRAY); // Deshabilitar array de vértices
	
	glPopMatrix(); // Restaurar estado

	glEnable(GL_LIGHTING); // Reactivar iluminación
}

// Proyectar punto 3D a coordenadas de pantalla 2D
bool proyectarAPantalla(const GLdouble model[16],// Matriz de modelo
	const GLdouble proj[16], // Matriz de proyección
	const GLint viewport[4], // Viewport
	float x, float y, float z, // Punto 3D
	float& sx, float& sy) // Punto 2D resultante
{
	// Convertir punto 3D a coordenadas homogéneas
	double vx = x, vy = y, vz = z, vw = 1.0;

	// Aplicar matriz de modelo
	double mx = model[0] * vx + model[4] * vy + model[8] * vz + model[12] * vw; // X
	double my = model[1] * vx + model[5] * vy + model[9] * vz + model[13] * vw; // Y
	double mz = model[2] * vx + model[6] * vy + model[10] * vz + model[14] * vw; // Z
	double mw = model[3] * vx + model[7] * vy + model[11] * vz + model[15] * vw; // W

	// Aplicar matriz de proyección
	double px = proj[0] * mx + proj[4] * my + proj[8] * mz + proj[12] * mw; // X
	double py = proj[1] * mx + proj[5] * my + proj[9] * mz + proj[13] * mw; // Y
	double pz = proj[2] * mx + proj[6] * my + proj[10] * mz + proj[14] * mw; // Z
	double pw = proj[3] * mx + proj[7] * my + proj[11] * mz + proj[15] * mw; // W
	// Verificar división por cero
	if (pw == 0.0) 
		return false;// No se puede proyectar

	// Convertir a coordenadas normalizadas
	px /= pw; // Normalizar X
	py /= pw; // Normalizar Y
	pz /= pw; // Normalizar Z

	// Verificar si el punto está dentro del volumen de recorte
	if (pz < -1.0 || pz > 1.0)
		return false; // Fuera del volumen de recorte
	// Convertir a coordenadas de ventana
	sx = (float)(viewport[0] + (px + 1.0) * 0.5 * viewport[2]); // X en pantalla
	sy = (float)(viewport[1] + (py + 1.0) * 0.5 * viewport[3]); // Y en pantalla
	
	return true; // Proyección exitosa
}

// Dibujar etiquetas de las partes del modelo
void dibujarEtiquetasPartes()
{
	// Obtener matrices y viewport actuales
	GLdouble model[16], proj[16]; // Matrices
	GLint viewport[4]; // Viewport
	
	glGetDoublev(GL_MODELVIEW_MATRIX, model); // Obtener matriz de modelo
	glGetDoublev(GL_PROJECTION_MATRIX, proj); // Obtener matriz de proyección
	glGetIntegerv(GL_VIEWPORT, viewport); // Obtener viewport

	// Definir etiquetas 3D
	struct Label3D {
		const char* texto; // Texto de la etiqueta
		float x, y, z; // Posición 3D
		float r, g, b; // Color
	};
	// Lista de etiquetas
	std::vector<Label3D> labels;

	// Etiquetas de los pulmones
	labels.push_back({ "Pulmon izquierdo",  1.8f, -1.0f,  0.5f,   1.0f, 1.0f, 0.6f }); // Pulmón izquierdo
	labels.push_back({ "Pulmon derecho",   -1.8f, -1.0f,  0.5f,   1.0f, 1.0f, 0.6f }); // Pulmón derecho

	// Etiquetas de la tráquea y bronquios
	labels.push_back({ "Traquea",     0.0f,  1.5f,  0.0f,   0.8f, 0.9f, 1.0f }); // Tráquea
	labels.push_back({ "Bronquios",   0.0f, -0.2f,  0.0f,   0.8f, 0.9f, 1.0f }); // Bronquios
	labels.push_back({ "Bronquiolos", 0.0f, -1.5f,  0.0f,   0.8f, 0.9f, 1.0f }); // Bronquiolos

	// Etiquetas de la columna y costillas (si se muestran)
	if (mostrarCostillas)
	{
		labels.push_back({ "Columna vertebral", 0.0f,  0.0f, -2.7f,   0.9f, 0.9f, 0.9f }); // Columna vertebral
		labels.push_back({ "Costillas",         0.0f,  0.8f, -1.5f,   0.9f, 0.9f, 0.9f }); // Costillas
	}

	// Etiqueta del torso (si no es translúcido)
	if (!torsoTranslucido)
	{
		labels.push_back({ "Tejido blando / Torso", 0.0f, -0.5f,  1.0f,   1.0f, 0.8f, 0.7f }); // Torso
	}

	// Proyectar etiquetas 3D a 2D
	struct Label2D {
		const char* texto; // Texto de la etiqueta
		float sx, sy; // Posición en pantalla
		float r, g, b; // Color
		bool visible; // Visibilidad
	};

	std::vector<Label2D> labels2D(labels.size()); // Vector para etiquetas 2D
	// Proyectar cada etiqueta
	for (size_t i = 0; i < labels.size(); ++i)
	{
		float sx, sy; // Coordenadas en pantalla
		// Proyectar punto 3D a 2D
		bool ok = proyectarAPantalla(
			model, proj, viewport, 
			labels[i].x, labels[i].y, labels[i].z,
			sx, sy
		);

		// Convertir coordenada Y a sistema de pantalla
		float syPantalla = (float)ALTO_VENTANA - sy;
		// Almacenar etiqueta 2D
		labels2D[i].texto = labels[i].texto; // Texto
		labels2D[i].sx = sx; // Coordenada X en pantalla
		labels2D[i].sy = syPantalla; // Coordenada Y en pantalla
		labels2D[i].r = labels[i].r; // Color R
		labels2D[i].g = labels[i].g; // Color G
		labels2D[i].b = labels[i].b; // Color B
		labels2D[i].visible = ok; // Visibilidad
	}

	// Configurar proyección ortográfica para texto 2D
	glMatrixMode(GL_PROJECTION); // Cambiar a proyección
	glPushMatrix(); // Guardar matriz
	glLoadIdentity(); // Cargar identidad
	glOrtho(0, ANCHO_VENTANA, ALTO_VENTANA, 0, -1, 1); // Proyección ortográfica
	// Cambiar a modelo-vista
	glMatrixMode(GL_MODELVIEW); // Cambiar a modelo-vista
	glPushMatrix(); // Guardar matriz
	glLoadIdentity(); // Cargar identidad

	glDisable(GL_DEPTH_TEST); // Desactivar prueba de profundidad

	// Dibujar cada etiqueta visible
	for (size_t i = 0; i < labels2D.size(); ++i)
	{
		if (!labels2D[i].visible) continue; // Saltar si no es visible

		// Dibujar texto en pantalla
		dibujarTexto2D(
			labels2D[i].texto, // Texto
			labels2D[i].sx, // Coordenada X
			labels2D[i].sy, // Coordenada Y
			labels2D[i].r, // Color R
			labels2D[i].g, // Color G
			labels2D[i].b // Color B
		);
	}
	// Reactivar prueba de profundidad
	glEnable(GL_DEPTH_TEST);

	// Restaurar matrices
	glPopMatrix(); //Restaurar modelo-vista        
	glMatrixMode(GL_PROJECTION); // Cambiar a proyección
	glPopMatrix(); // Restaurar proyección
	glMatrixMode(GL_MODELVIEW); // Volver a modelo-vista
}

// Dibujar leyenda de controles en pantalla
void dibujarLeyendaTeclas()
{
	// Configurar proyección ortográfica para texto 2D
	glMatrixMode(GL_PROJECTION); // Cambiar a proyección
	glPushMatrix(); // Guardar matriz
	glLoadIdentity(); // Cargar identidad
	glOrtho(0, ANCHO_VENTANA, ALTO_VENTANA, 0, -1, 1); // Proyección ortográfica
	// Cambiar a modelo-vista
	glMatrixMode(GL_MODELVIEW); // Cambiar a modelo-vista
	glPushMatrix(); // Guardar matriz
	glLoadIdentity(); // Cargar identidad

	// Desactivar prueba de profundidad
	glDisable(GL_DEPTH_TEST);

	// Posición inicial para el texto
	float x = 10.0f; // Margen izquierdo
	float y = 20.0f; // Margen superior

	// Color del texto
	float r = 1.0f, g = 1.0f, b = 1.0f;

	// Dibujar líneas de texto
	dibujarTexto2D("Controles:", x, y, r, g, b);
	y += 18.0f;  
	// Instrucciones de control
	dibujarTexto2D("Flechas : Rotar camara", x, y, r, g, b);	y += 18.0f; 
	dibujarTexto2D("W / S   : Zoom acercar/alejar", x, y, r, g, b); 	y += 18.0f; 
	dibujarTexto2D("Numero 1: Pulmon izquierdo transparente", x, y, r, g, b);	y += 18.0f; 
	dibujarTexto2D("Numero 2: Pulmon derecho transparente", x, y, r, g, b);	y += 18.0f; 
	dibujarTexto2D("Numero 3: Mostrar/Ocultar costillas + columna", x, y, r, g, b); 	y += 18.0f;
	dibujarTexto2D("Numero 4: Torso translucido / opaco", x, y, r, g, b);	y += 18.0f;
	// Restaurar matrices
	glEnable(GL_DEPTH_TEST);

	// Restaurar matrices
	glPopMatrix(); //Restaurar modelo-vista       
	glMatrixMode(GL_PROJECTION); // Cambiar a proyección
	glPopMatrix(); // Restaurar proyección
	glMatrixMode(GL_MODELVIEW); // Volver a modelo-vista
}


// --- MAIN (PUNTO DE ENTRADA) ---
// Función Principal
int main(void)
{
	GLFWwindow* window; // Ventana GLFW

    /* Inicializar Librería */
	if (!glfwInit()) return -1; // Fallo al inicializar GLFW

    /* Crear Ventana y Contexto */
	window = glfwCreateWindow(ANCHO_VENTANA, ALTO_VENTANA, "Torax 3D - Grupo 01 COGRAVI - UPN", NULL, NULL); // Crear ventana
	if (!window) { glfwTerminate(); return -1; } // Fallo al crear ventana

	glfwMakeContextCurrent(window); // Hacer contexto actual

    /* Inicializar OpenGL */
	if (!inicializarOpenGL()) { glfwTerminate(); return -1; } // Fallo al inicializar OpenGL

    // Configurar Callbacks
	glfwSetFramebufferSizeCallback(window, callbackCambioTamano); // Cambio de tamaño de ventana

    // Configuración Inicial de Vista
	configurarProyeccion(ANCHO_VENTANA, ALTO_VENTANA); // Configurar proyección inicial

    /* Bucle Principal */
    while (!glfwWindowShouldClose(window))
    {
		// 1. Procesar Entrada del usuario
        procesarEntrada(window);

        // 2. Actualizar Lógica (Animación)
		double tiempo = glfwGetTime(); // Tiempo
		factorRespiracion = 1.0f + 0.05f * sin(tiempo * 2.0f); // Factor de respiración

        // 3. Renderizar
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Limpiar buffers
		// Configurar cámara
		glMatrixMode(GL_MODELVIEW); // Cambiar a modelo-vista
		glLoadIdentity(); // Resetear matriz
		glTranslatef(0.0f, 0.0f, -camDist); // Alejar cámara
		glRotatef(camPitch, 1.0f, 0.0f, 0.0f); // Rotar cámara vertical
		glRotatef(camYaw, 0.0f, 1.0f, 0.0f); // Rotar cámara horizontal
		// Dibujar modelo completo
		dibujarModeloCompleto(); // Dibujar escena
		dibujarEtiquetasPartes(); // Dibujar etiquetas
		dibujarLeyendaTeclas(); // Dibujar leyenda de controles

        // 4. Intercambiar Buffers y Eventos
		glfwSwapBuffers(window); // Intercambiar buffers
		glfwPollEvents(); // Procesar eventos
    }
	// Finalizar
    glfwTerminate();
	return 0; // Éxito
}