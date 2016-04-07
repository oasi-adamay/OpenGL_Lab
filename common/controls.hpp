#ifndef CONTROLS_HPP
#define CONTROLS_HPP

class GlfwControls{
private:
	GLFWwindow* window;
	glm::mat4 ViewMatrix;
	glm::mat4 ProjectionMatrix;

	// Initial position : on +Z
	glm::vec3 position;		

	// Initial horizontal angle : toward -Z
	float horizontalAngle;
	// Initial vertical angle : none
	float verticalAngle;
	// Initial Field of View
	float initialFoV;

	float speed;
	float mouseSpeed;


public:
	GlfwControls(GLFWwindow* _window);

	void computeMatricesFromInputs();
	glm::mat4 getViewMatrix();
	glm::mat4 getProjectionMatrix();
	glm::vec3 getCameraPosition();
	void setCameraPosition(vec3 _position);


};



#endif