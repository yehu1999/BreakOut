//本文件不可运行，请从生成中排除
#include<mythings/shader.h>
#include<mythings/camera.h>
#include<mythings/model.h>
#include<mythings/debug.h>
#include<mythings/render.h>
#include<mythings/texture.h>
#include<mythings/game/game.h>
#include<mythings/resource_manager.h>
#include<tuple>

//准备工作
namespace SettingUp
{

	//在开始真正写游戏机制之前，我们首先需要配置一个简单的框架，用来存放这个游戏，
	//这个游戏将会用到几个第三方库，它们的大多数都已经在前面的教程中介绍过了。
	//在需要用到新的库的时候，我会作出适当的介绍。

	//首先，我们定义一个所谓的超级(Uber)游戏类，它会包含所有相关的渲染和游戏代码。这个游戏类的主要作用是（简单）管理你的游戏代码，并与此同时将所有的窗口代码从游戏中解耦。
	//这样子的话，你就可以把相同的类迁移到完全不同的窗口库（比如SDL或SFML）而不需要做太多的工作。

	//抽象并归纳游戏或图形代码至类与对象中有成千上万种方式。
	//在这个系列教程中你所看到的仅是其中的一种。
	//如果你觉得能有更好的方式进行实现，你可以尝试改进我的这个实现。

	//这个游戏类封装了一个初始化函数、一个更新函数、一个处理输入函数以及一个渲染函数：
	class Game
	{
	public:
		// 游戏状态
		GameState  State;
		GLboolean  Keys[1024];
		GLuint     Width, Height;
		// 构造函数/析构函数
		Game(GLuint width, GLuint height);
		~Game();
		// 初始化游戏状态（加载所有的着色器/纹理/关卡）
		void Init();
		// 游戏循环
		void ProcessInput(GLfloat dt);
		void Update(GLfloat dt);
		void Render();
	};

	//这个类应该包含了所有在一个游戏类中会出现的东西。
	//我们通过给定一个宽度和高度（对应于你玩游戏时的分辨率）来初始化这个游戏，
	//并且使用Init函数来加载着色器、纹理并且初始化所有的游戏状态。
	//我们可以通过调用ProcessInput函数，并使用存储在Keys数组里的数据来处理输入。
	//并且在Update函数里面我们可以更新游戏设置状态（比如玩家/球的移动）。
	//最后，我们还可以调用Render函数来对游戏进行渲染。
	//注意，我们将运动逻辑与渲染逻辑分开了。

	//这个Game类同样了封装了一个叫做State的变量，它的类型是GameState，定义如下：
	//代表了游戏的当前状态
	enum GameState
	{
		GAME_ACTIVE,
		GAME_MENU,
		GAME_WIN
	};

	//这个类可以帮助我们跟踪游戏的当前状态。
	//这样的话我们就可以根据当前游戏的状态来决定渲染和 / 或者处理不同的元素(Item)了
	//（比如当我们在游戏菜单界面的时候就可能需要渲染和处理不同的元素了）。

	//工具类-------------------------------------------------------------------------------------------------

	//因为我们正在开发一个大型应用，所以我们将不得不频繁地重用一些 OpenGL 的概念，比如纹理和着色器等。
	//因此，为这两个项目创建一个更加易用的接口也是情理之中的事了，
	//就像在我们前面教程中创建的那个着色器类一样。

	//着色器类会接受两个或三个（如果有几何着色器）字符串，并生成一个编译好的着色器（如果失败的话则生成错误信息）。
	//这个着色器类也包含很多工具 (Utility) 函数来帮助快速设置 uniform 值。
	//纹理类会接受一个字节 (Byte) 数组以及宽度和高度，并（根据设定的属性）生成一个 2D 纹理图像。
	//同样，这个纹理类也会封装一些工具函数。

	//我们并不会深入讨论这些类的实现细节，因为学到这里你应该可以很容易地理解它们是如何工作的了。
	//出于这个原因，你可以在下面找到它们的头文件和代码文件，都有详细的注释：
	//着色器 shader.h      (使用自己的版本)
	//纹理   texture.h

	//注意当前的纹理类仅是为 2D 纹理设计的，但你很容易就可以将其扩张至更多的纹理类型。

	//资源管理------------------------------------------------------------------------------------------------

	//尽管着色器与纹理类的函数本身就很棒了，它们仍需要有一个字节数组或一些字符串来调用它们。
	//我们可以很容易将文件加载代码嵌入到它们自己的类中，但这稍微有点违反了单一功能原则 (Single Responsibility Principle)，
	//即这两个类应当分别仅仅关注纹理或者着色器本身，而不是它们的文件加载机制。

	//出于这个原因，我们通常会用一个更加有组织的方法（译注：来实现文件的加载），
	//就是创建一个所谓资源管理器的实体，专门加载游戏相关的资源。
	//创建一个资源管理器有多种方法。
	//在这个教程中我们选择使用一个单一实例 (Singleton) 的静态资源管理器，
	//（由于它静态的本质）它在整个工程中都可以使用，它会封装所有的已加载资源以及一些相关的加载功能。

	//使用一个具有静态属性的单一实例类有很多优点也有很多缺点。
	//它主要的缺点就是这样会损失 OOP 属性，并且丧失构造与析构的控制。
	//不过，对于我们这种小项目来说是这些问题也是很容易处理的。

	//和其它类的文件一样，这个资源管理器的代码如下：
	//资源管理器  resource_manager.h
	void how()
	{
		//通过使用资源管理器，我们可以很容易地把着色器加载到程序里面：
		Shader shader = ResourceManager::LoadShader("vertex.vs", "fragment.vs", nullptr, "test");
		// 接下来使用它
		shader.use();
		// 或者
		ResourceManager::GetShader("test").use();
	}

	//Game 类、资源管理器类，以及很容易管理的 Shader 和 Texture2D 类一起组成了之后教程的基础，
	//我们之后会广泛使用这些类来实现我们的 Breakout 游戏。

	//程序----------------------------------------------------------------------------------------------------

	//我们仍然需要为这个游戏创建一个窗口并且设置一些 OpenGL 的初始状态。
	//我们确保使用 OpenGL 的面剔除功能和混合功能。
	//我们不需要使用深度测试，因为这个游戏完全是 2D 的，所有顶点都有相同的 z 值，
	//所以开启深度测试并没有什么用，反而可能造成深度冲突 (Z-fighting)。

	//这个 Breakout 游戏的起始代码非常简单：
	//我们用 GLFW 创建一个窗口，注册一些回调函数，创建一个 Game 对象，并将所有相关的信息都传到游戏类中。
	//代码如下：
	// GLFW function declerations
	void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);

	// The Width of the screen
	const GLuint SCREEN_WIDTH = 800;
	// The height of the screen
	const GLuint SCREEN_HEIGHT = 600;

	Game Breakout(SCREEN_WIDTH, SCREEN_HEIGHT);

	int main(int argc, char* argv[])
	{
		glfwInit();
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

		GLFWwindow* window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Breakout", nullptr, nullptr);
		glfwMakeContextCurrent(window);

		//glad加载  //原教程使用的是glew
		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
		{
			std::cout << "Failed to initialize GLAD" << std::endl;
			return -1;
		}

		glfwSetKeyCallback(window, key_callback);

		// OpenGL configuration
		glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
		glEnable(GL_CULL_FACE);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// Initialize game
		Breakout.Init();

		// DeltaTime variables
		GLfloat deltaTime = 0.0f;
		GLfloat lastFrame = 0.0f;

		// Start Game within Menu State
		Breakout.State = GAME_ACTIVE;

		while (!glfwWindowShouldClose(window))
		{
			// Calculate delta time
			GLfloat currentFrame = glfwGetTime();
			deltaTime = currentFrame - lastFrame;
			lastFrame = currentFrame;
			glfwPollEvents();

			//deltaTime = 0.001f;
			// Manage user input
			Breakout.ProcessInput(deltaTime);

			// Update Game state
			Breakout.Update(deltaTime);

			// Render
			glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT);
			Breakout.Render();

			glfwSwapBuffers(window);
		}

		// Delete all resources as loaded using the resource manager
		ResourceManager::Clear();

		glfwTerminate();
		return 0;
	}

	void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
	{
		// When a user presses the escape key, we set the WindowShouldClose property to true, closing the application
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
			glfwSetWindowShouldClose(window, GL_TRUE);
		if (key >= 0 && key < 1024)
		{
			if (action == GLFW_PRESS)
				Breakout.Keys[key] = GL_TRUE;
			else if (action == GLFW_RELEASE)
				Breakout.Keys[key] = GL_FALSE;
		}
	}

	//运行这个代码，你应该能得到下面的输出：
	// https://learnopengl-cn.github.io/img/06/Breakout/02/setting-up.png

	//现在我们已经为之后的教程构建了一个坚实的框架，我们将不断地拓展这个游戏类，封装新的功能。
	//如果你准备好了，就可以开始下一节的学习了。
}

//渲染精灵
namespace RenderingSprites
{
	//为了给我们当前这个黑漆漆的游戏世界带来一点生机，
	//我们将会渲染一些精灵(Sprite)来填补这些空虚。
	//精灵有很多种定义，但这里主要是指一个2D图片，
	//它通常是和一些摆放相关的属性数据一起使用，
	//比如位置、旋转角度以及二维的大小。
	//简单来说，精灵就是那些可以在2D游戏中渲染的图像/纹理对象。

	//我们可以像前面大多数教程里做的那样，用顶点数据创建2D形状，将所有数据传进GPU并手动变换图形。
	//然而，在我们这样的大型应用中，我们最好是对2D形状渲染做一些抽象化。
	//如果我们要对每一个对象手动定义形状和变换的话，很快就会变得非常凌乱了。

	//在这个教程中，我们将会定义一个渲染类，让我们用最少的代码渲染大量的精灵。
	//这样，我们就可以从散沙一样的OpenGL渲染代码中抽象出游戏代码，
	//这也是在一个大型工程中常用的做法。
	//虽然我们首先还要去配置一个合适的投影矩阵。

	//2D投影矩阵--------------------------------------------------------------------------------

	//从这个坐标系统教程中，我们明白了投影矩阵的作用是把观察空间坐标转化为标准化设备坐标。
	//通过生成合适的投影矩阵，我们就可以在不同的坐标系下计算，
	//这可能比把所有的坐标都指定为标准化设备坐标（再计算）要更容易处理。

	//我们不需要对坐标系应用透视，因为这个游戏完全是2D的，
	//所以一个正射投影矩阵(Orthographic Projection Matrix)就可以了。
	//由于正射投影矩阵几乎直接变换所有的坐标至裁剪空间，
	//我们可以定义如下的投影矩阵指定世界坐标为屏幕坐标：
	glm::mat4 projection = glm::ortho(0.0f, 800.0f, 600.0f, 0.0f, -1.0f, 1.0f);
	//前面的四个参数依次指定了投影平截头体的左、右、下、上边界。
	//后面两个参数分别对应近视点，远视点。
	//这个投影矩阵可以把所有在0到800之间的x坐标变换到-1到1之间，
	//并把所有在0到600之间的y坐标变换到-1到1之间。
	//这里我们指定了平截头体顶部的y坐标值为0，底部的y坐标值为600。
	//所以，这个场景的左上角坐标为(0,0)，右下角坐标为(800,600)，就像屏幕坐标那样。
	//观察空间坐标直接对应最终像素的坐标：https://learnopengl-cn.github.io/img/06/Breakout/03/projection.png

	//这样我们就可以指定所有的顶点坐标为屏幕上的像素坐标了，这对2D游戏来说相当直观。

	//渲染精灵-----------------------------------------------------------------------------------

	//渲染一个实际的精灵应该不会太复杂。
	//我们创建一个有纹理的四边形，它在之后可以使用一个模型矩阵来变换，
	//然后我们会用之前定义的正射投影矩阵来投影它。

	//由于Breakout是一个静态的游戏，这里不需要观察/视图/摄像机矩阵，
	//我们可以直接使用投影矩阵把世界空间坐标变换到裁剪空间坐标。

	//为了变换精灵我们会使用下面这个顶点着色器：
	/*
	#version 330 core
	layout (location = 0) in vec4 vertex; // <vec2 position, vec2 texCoords>

	out vec2 TexCoords;

	uniform mat4 model;
	uniform mat4 projection;

	void main()
	{
		TexCoords = vertex.zw;
		gl_Position = projection * model * vec4(vertex.xy, 0.0, 1.0);
	}
	*/
	//注意，我们仅用了一个vec4变量来存储位置和纹理坐标数据。
	//因为位置和纹理坐标数据都只包含了两个float，
	//所以我们可以把他们组合在一起作为一个单一的顶点属性。

	//片段着色器也比较直观。
	//我们会在这里获取一个纹理和一个颜色向量，它们都会影响片段的最终颜色。
	//有了这个uniform颜色向量，我们就可以很方便地在游戏代码中改变精灵的颜色了。
	/*
	#version 330 core
	in vec2 TexCoords;
	out vec4 color;

	uniform sampler2D image;
	uniform vec3 spriteColor;

	void main()
	{
		color = vec4(spriteColor, 1.0) * texture(image, TexCoords);
	}
	*/

	//为了让精灵的渲染更加有条理，我们定义了一个SpriteRenderer类， （已加入render.h）
	//有了它只需要一个函数就可以渲染精灵了。它的定义如下：
	class SpriteRenderer
	{
	public:
		SpriteRenderer(Shader& shader);
		~SpriteRenderer();

		void DrawSprite(Texture2D& texture, glm::vec2 position,
			glm::vec2 size = glm::vec2(10, 10), GLfloat rotate = 0.0f,
			glm::vec3 color = glm::vec3(1.0f));
	private:
		Shader shader;
		GLuint quadVAO;

		void initRenderData();
	};

	//SpriteRenderer类封装了一个着色器对象，一个顶点数组对象以及一个渲染和初始化函数。
	//它的构造函数接受一个着色器对象用于之后的渲染。

	//初始化-------------------------------------------------------------------------------------

	//首先，让我们深入研究一下负责配置quadVAO的initRenderData函数：
	void SpriteRenderer::initRenderData()
	{
		// 配置 VAO/VBO
		GLuint VBO;
		GLfloat vertices[] =
		{
			// 位置     // 纹理
			0.0f, 1.0f, 0.0f, 1.0f,
			1.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 0.0f,

			0.0f, 1.0f, 0.0f, 1.0f,
			1.0f, 1.0f, 1.0f, 1.0f,
			1.0f, 0.0f, 1.0f, 0.0f
		};

		glGenVertexArrays(1, &this->quadVAO);
		glGenBuffers(1, &VBO);

		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

		glBindVertexArray(this->quadVAO);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}

	//这里，我们首先定义了一组以四边形的左上角为(0,0)坐标的顶点。
	//这意味着当我们在四边形上应用一个位移或缩放变换的时候，它们会从四边形的左上角开始进行变换。
	//这在2D图形以及/或GUI系统中广为接受，元素的位置定义为元素左上角的位置。

	//接下来我们简单地向GPU传递顶点数据，并且配置顶点属性，当然在这里仅有一个顶点属性。
	//因为所有的精灵共享着同样的顶点数据，我们只需要为这个精灵渲染器定义一个VAO就行了。

	//渲染--------------------------------------------------------------------------------------

	//渲染精灵并不是太难；
	//我们使用精灵渲染器的着色器，配置一个模型矩阵并且设置相关的uniform。
	//这里最重要的就是变换的顺序：

	void SpriteRenderer::DrawSprite(Texture2D& texture, glm::vec2 position,
		glm::vec2 size, GLfloat rotate, glm::vec3 color)
	{
		// 准备变换
		this->shader.use();
		glm::mat4 model;
		model = glm::translate(model, glm::vec3(position, 0.0f));

		model = glm::translate(model, glm::vec3(0.5f * size.x, 0.5f * size.y, 0.0f));
		model = glm::rotate(model, rotate, glm::vec3(0.0f, 0.0f, 1.0f));
		model = glm::translate(model, glm::vec3(-0.5f * size.x, -0.5f * size.y, 0.0f));

		model = glm::scale(model, glm::vec3(size, 1.0f));

		this->shader.setMat4("model", model);
		this->shader.setVec3("spriteColor", color);

		glActiveTexture(GL_TEXTURE0);
		texture.Bind();

		glBindVertexArray(this->quadVAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glBindVertexArray(0);
	}

	//当试图在一个场景中用旋转矩阵和缩放矩阵放置一个对象的时候，
	//建议是首先做缩放变换，再旋转，最后才是位移变换。
	//因为矩阵乘法是从右向左执行的，所以我们变换的矩阵顺序是相反的：移动，旋转，缩放。

	//旋转变换可能看起来稍微有点让人望而却步。
	//我们从变换教程里面知道旋转总是围绕原点(0,0)转动的。
	//因为我们指定了四边形的左上角为(0,0)，所有的旋转都会围绕这个(0,0)。
	//简单来说，在四边形左上角的旋转原点(Origin of Rotation)会产生不想要的结果。
	//我们想要做的是把旋转原点移到四边形的中心，这样旋转就会围绕四边形中心而不是左上角了。
	//我们会在旋转之前把旋转原点移动到四边形中心来解决这个问题。
	// https://learnopengl-cn.github.io/img/06/Breakout/03/rotation-origin.png

	//因为我们首先会缩放这个四边形，我们在位移精灵的中心时还需要把精灵的大小考虑进来（这也是为什么我们乘以了精灵的size向量）。
	//在旋转变换应用之后，我们会反转之前的平移操作。

	//把所有变换组合起来我们就能以任何想要的方式放置、缩放并平移每个精灵了。
	//下面你可以找到精灵渲染器完整的源代码：
	// （已加入render.h）

	//你好，精灵--------------------------------------------------------------------------------

	//有了SpriteRenderer类，我们终于能够渲染实际的图像到屏幕上了！
	//让我们来在游戏代码里面初始化一个精灵并且加载我们最喜爱的纹理：
	/*
	SpriteRenderer* Renderer;

	void Game::Init()
	{
		// 加载着色器
		ResourceManager::LoadShader("shaders/sprite.vs", "shaders/sprite.frag", nullptr, "sprite");
		// 配置着色器
		glm::mat4 projection = glm::ortho(0.0f, static_cast<GLfloat>(this->Width),
			static_cast<GLfloat>(this->Height), 0.0f, -1.0f, 1.0f);
		ResourceManager::GetShader("sprite").use().setInt("image", 0);
		ResourceManager::GetShader("sprite").setMat4("projection", projection);
		// 设置专用于渲染的控制
		Renderer = new SpriteRenderer(ResourceManager::GetShader("sprite"));
		// 加载纹理
		ResourceManager::LoadTexture("textures/awesomeface.png", GL_TRUE, "face");
	}
	*/

	//这里我们把精灵放置在靠近屏幕中心的位置，它的高度会比宽度大一点。
	//我们同样也把它旋转了45度并把它设置为绿色。
	//注意，我们设定的精灵的位置是精灵四边形左上角的位置。

	//如果你一切都做对了你应该可以看到下面的结果：https://learnopengl-cn.github.io/img/06/Breakout/03/rendering-sprites.png

	//现在我们已经让渲染系统正常工作了，我们可以在下一节教程中用它来构建游戏的关卡。
}

//关卡
namespace Levels
{
	//Breakout不会只是一个单一的绿色笑脸，而是一些由许多彩色砖块组成的完整关卡。
	//我们希望这些关卡有以下特性：
	//他们足够灵活以便于支持任意数量的行或列、
	//可以拥有不可摧毁的坚固砖块、支持多种类型的砖块且这些信息被存储在外部文件中。

	//在本教程中，我们将简要介绍用于管理大量砖块的游戏关卡对象的代码，
	//首先我们需要先定义什么是一个砖块。

	//我们创建一个被称为游戏对象的组件作为一个游戏内物体的基本表示。
	//这样的游戏对象持有一些状态数据，如其位置、大小与速率。
	//它还持有颜色、旋转、是否坚硬(不可被摧毁)、是否被摧毁的属性，
	//除此之外，它还存储了一个Texture2D变量作为其精灵(Sprite)。

	//游戏中的每个物体都可以被表示为GameObject或这个类的派生类，
	//你可以在下面找到GameObject的代码：
	//gameObject.h

	//Breakout中的关卡基本由砖块组成，因此我们可以用一个砖块的集合表示一个关卡。
	//因为砖块需要和游戏对象几乎相同的状态，所以我们将关卡中的每个砖块表示为GameObject。
	//GameLevel类的布局如下所示：
	/*
	class GameLevel
	{
	public:
		std::vector<GameObject> Bricks;

		GameLevel() { }
		// 从文件中加载关卡
		void Load(const GLchar *file, GLuint levelWidth, GLuint levelHeight);
		// 渲染关卡
		void Draw(SpriteRenderer &renderer);
		// 检查一个关卡是否已完成 (所有非坚硬的瓷砖均被摧毁)
		GLboolean IsCompleted();
	private:
		// 由砖块数据初始化关卡
		void init(std::vector<std::vector<GLuint>> tileData, GLuint levelWidth, GLuint levelHeight);
	};
	*/

	//由于关卡数据从外部文本中加载，所以我们需要提出某种关卡的数据结构，
	//以下是关卡数据在文本文件中可能的表示形式的一个例子：
	/*
	1 1 1 1 1 1
	2 2 0 0 2 2
	3 3 4 4 3 3
	*/

	//在这里一个关卡被存储在一个矩阵结构中，每个数字代表一种类型的砖块，并以空格分隔。
	//在关卡代码中我们可以假定每个数字代表什么：
	// 数字0：无砖块，表示关卡中空的区域
	// 数字1：一个坚硬的砖块，不可被摧毁
	// 大于1的数字：一个可被摧毁的砖块，不同的数字区分砖块的颜色

	//上面的示例关卡在被GameLevel处理后，看起来会像这样：

	//GameLevel类使用两个函数从文件中生成一个关卡。
	//它首先将所有数字在Load函数中加载到二维容器(vector)里，
	//然后在init函数中处理这些数字，以创建所有的游戏对象。
	/*
	void GameLevel::Load(const GLchar *file, GLuint levelWidth, GLuint levelHeight)
	{
		// 清空过期数据
		this->Bricks.clear();
		// 从文件中加载
		GLuint tileCode;
		GameLevel level;
		std::string line;
		std::ifstream fstream(file);
		std::vector<std::vector<GLuint>> tileData;
		if (fstream)
		{
			while (std::getline(fstream, line)) // 读取关卡文件的每一行
			{
				std::istringstream sstream(line);
				std::vector<GLuint> row;
				while (sstream >> tileCode) // 读取被空格分隔的每个数字
					row.push_back(tileCode);
				tileData.push_back(row);
			}
			if (tileData.size() > 0)
				this->init(tileData, levelWidth, levelHeight);
		}
	}
	*/

	//被加载后的tileData数据被传递到GameLevel的init函数：
	/*
	void GameLevel::init(std::vector<std::vector<GLuint>> tileData, GLuint lvlWidth, GLuint lvlHeight)
	{
		// 计算每个维度的大小
		GLuint height = tileData.size();
		GLuint width = tileData[0].size();
		GLfloat unit_width = lvlWidth / static_cast<GLfloat>(width);
		GLfloat unit_height = lvlHeight / height;
		// 基于tileDataC初始化关卡
		for (GLuint y = 0; y < height; ++y)
		{
			for (GLuint x = 0; x < width; ++x)
			{
				// 检查砖块类型
				if (tileData[y][x] == 1)
				{
					glm::vec2 pos(unit_width * x, unit_height * y);
					glm::vec2 size(unit_width, unit_height);
					GameObject obj(pos, size,
						ResourceManager::GetTexture("block_solid"),
						glm::vec3(0.8f, 0.8f, 0.7f)
					);
					obj.IsSolid = GL_TRUE;
					this->Bricks.push_back(obj);
				}
				else if (tileData[y][x] > 1)
				{
					glm::vec3 color = glm::vec3(1.0f); // 默认为白色
					if (tileData[y][x] == 2)
						color = glm::vec3(0.2f, 0.6f, 1.0f);
					else if (tileData[y][x] == 3)
						color = glm::vec3(0.0f, 0.7f, 0.0f);
					else if (tileData[y][x] == 4)
						color = glm::vec3(0.8f, 0.8f, 0.4f);
					else if (tileData[y][x] == 5)
						color = glm::vec3(1.0f, 0.5f, 0.0f);

					glm::vec2 pos(unit_width * x, unit_height * y);
					glm::vec2 size(unit_width, unit_height);
					this->Bricks.push_back(
						GameObject(pos, size, ResourceManager::GetTexture("block"), color)
					);
				}
			}
		}
	}
	*/
	int hello;

	//init函数遍历每个被加载的数字，处理后将一个相应的GameObject添加到关卡的容器中。
	//每个砖块的尺寸(unit_width和unit_height)根据砖块的总数被自动计算以便于每块砖可以完美地适合屏幕边界。

	//在这里我们用两个新的纹理加载游戏对象，分别为block纹理与solid block纹理。
	// https://learnopengl-cn.github.io/img/06/Breakout/04/block-textures.png

	//这里有一个很好的小窍门，即这些纹理是完全灰度的。
	//其效果是，我们可以在游戏代码中，通过将灰度值与定义好的颜色矢量相乘来巧妙地操纵它们的颜色，
	//就如同我们在SpriteRenderer中所做的那样。
	//这样一来，自定义的颜色/外观就不会显得怪异或不平衡。

	//GameLevel类还包含一些其他的功能，比如渲染所有未被破坏的砖块，或验证是否所有的可破坏砖块均被摧毁。
	//你可以在下面找到GameLevel类的源码：
	//gameLevel.h

	//因为支持任意数量的行和列，这个游戏关卡类给我们带来了很大的灵活性，用户可以通过修改关卡文件轻松创建自己的关卡。

	//游戏中----------------------------------------------------------------------

	//我们希望在Breakout游戏中支持多个关卡，因此我们将在Game类中添加一个持有GameLevel变量的容器。
	//同时我们还将存储当前的游戏关卡。
	/*
	class Game
	{
		[...]
		std::vector<GameLevel> Levels;
		GLuint                 Level;
		[...]
	};
	*/

	//这个教程的Breakout版本共有4个游戏关卡：
	/*
	Standard
	A few small gaps
	Space invader
	Bounce galore
	*/

	//然后Game类的init函数初始化每个纹理和关卡：
	/*
	void Game::Init()
	{
		[...]
		// 加载纹理
		ResourceManager::LoadTexture("textures/background.jpg", GL_FALSE, "background");
		ResourceManager::LoadTexture("textures/awesomeface.png", GL_TRUE, "face");
		ResourceManager::LoadTexture("textures/block.png", GL_FALSE, "block");
		ResourceManager::LoadTexture("textures/block_solid.png", GL_FALSE, "block_solid");
		// 加载关卡
		GameLevel one; one.Load("levels/one.lvl", this->Width, this->Height * 0.5);
		GameLevel two; two.Load("levels/two.lvl", this->Width, this->Height * 0.5);
		GameLevel three; three.Load("levels/three.lvl", this->Width, this->Height * 0.5);
		GameLevel four; four.Load("levels/four.lvl", this->Width, this->Height * 0.5);
		this->Levels.push_back(one);
		this->Levels.push_back(two);
		this->Levels.push_back(three);
		this->Levels.push_back(four);
		this->Level = 1;
	}
	*/

	//现在剩下要做的就是通过调用当前关卡的Draw函数来渲染我们完成的关卡，
	//然后使用给定的sprite渲染器调用每个GameObject的Draw函数。
	//除了关卡之外，我们还会用一个很好的背景图片来渲染这个场景：https://learnopengl.com/img/in-practice/breakout/textures/background.jpg
	/*
	void Game::Render()
	{
		if(this->State == GAME_ACTIVE)
		{
			// 绘制背景
			Renderer->DrawSprite(ResourceManager::GetTexture("background"),
				glm::vec2(0, 0), glm::vec2(this->Width, this->Height), 0.0f
			);
			// 绘制关卡
			this->Levels[this->Level].Draw(*Renderer);
		}
	}
	*/

	//结果便是如下这个被呈现的关卡，它使我们的游戏变得开始生动起来：
	// https://learnopengl-cn.github.io/img/06/Breakout/04/levels.png

	//玩家挡板------------------------------------------------------------------------------------------------

	//此时我们在场景底部引入一个由玩家控制的挡板，
	//挡板只允许水平移动，并且在它接触任意场景边缘时停止。
	//对于玩家挡板，我们将使用以下纹理：https://learnopengl-cn.github.io/img/06/Breakout/04/paddle.png

	//一个挡板对象拥有位置、大小、渲染纹理等属性，所以我们理所当然地将其定义为一个GameObject。
	/*
	// 初始化挡板的大小
	const glm::vec2 PLAYER_SIZE(100, 20);
	// 初始化当班的速率
	const GLfloat PLAYER_VELOCITY(500.0f);

	GameObject      *Player;

	void Game::Init()
	{
		[...]
		ResourceManager::LoadTexture("textures/paddle.png", true, "paddle");
		[...]
		glm::vec2 playerPos = glm::vec2(
			this->Width / 2 - PLAYER_SIZE.x / 2,
			this->Height - PLAYER_SIZE.y
		);
		Player = new GameObject(playerPos, PLAYER_SIZE, ResourceManager::GetTexture("paddle"));
	}
	*/
	int hi;

	//这里我们定义了几个常量来初始化挡板的大小与速率。
	//在Game的Init函数中我们计算挡板的初始位置，使其中心与场景的水平中心对齐。

	//除此之外我们还需要在Game的Render函数中添加：
	// 
	// Player->Draw(*Renderer);

	//如果你现在启动游戏，你不仅会看到关卡画面，还会有一个在场景底部边缘的奇特的挡板。
	//到目前为止，它除了静态地放置在那以外不会发生任何事情，
	//因此我们需要进入游戏的ProcessInput函数，使得当玩家按下A和D时，挡板可以水平移动。
	/*
	void Game::ProcessInput(GLfloat dt)
	{
		if (this->State == GAME_ACTIVE)
		{
			GLfloat velocity = PLAYER_VELOCITY * dt;
			// 移动挡板
			if (this->Keys[GLFW_KEY_A])
			{
				if (Player->Position.x >= 0)
					Player->Position.x -= velocity;
			}
			if (this->Keys[GLFW_KEY_D])
			{
				if (Player->Position.x <= this->Width - Player->Size.x)
					Player->Position.x += velocity;
			}
		}
	}
	*/

	//在这里，我们根据用户按下的键，向左或向右移动挡板(注意我们将速率与deltaTime相乘)。
	//当挡板的x值小于0，它将移动出游戏场景的最左侧，所以我们只允许挡板的x值大于0时向左移动。
	//对于右侧边缘我们做相同的处理，但我们必须比较场景的右侧边缘与挡板的右侧边缘，即场景宽度减去挡板宽度。

	//现在启动游戏，将呈现一个玩家可控制在整个场景底部自由移动的挡板。
	// https://learnopengl-cn.github.io/img/06/Breakout/04/levels-player.png
}

//碰撞-球
namespace aBall
{
	//此时我们已经有了一个包含有很多砖块和玩家的一个挡板的关卡。
	//与经典的Breakout内容相比还差一个球。
	//游戏的目的是让球撞击所有的砖块，直到所有的可销毁砖块都被销毁，
	//但同时也要满足条件：球不能碰触屏幕的下边缘。

	//除了通用的游戏对象组件，球还需要有半径和一个布尔值，
	//该布尔值用于指示球被固定(stuck)在玩家挡板上还是被允许自由运动的状态。
	//当游戏开始时，球被初始固定在玩家挡板上，直到玩家按下任意键开始游戏。

	//由于球只是一个附带了一些额外属性的GameObject，
	//所以按照常规需要创建BallObject类作为GameObject的子类。
	/*
	class BallObject : public GameObject
	{
		public:
			// 球的状态
			GLfloat   Radius;
			GLboolean Stuck;


			BallObject();
			BallObject(glm::vec2 pos, GLfloat radius, glm::vec2 velocity, Texture2D sprite);

			glm::vec2 Move(GLfloat dt, GLuint window_width);
			void      Reset(glm::vec2 position, glm::vec2 velocity);
	};
	*/

	//BallObject的构造函数不但初始化了其自身的值，而且实际上也潜在地初始化了GameObject。
	//BallObject类拥有一个Move函数，该函数用于根据球的速度来移动球，
	//并检查它是否碰到了场景的任何边界，如果碰到的话就会反转球的速度：
	/*
	glm::vec2 BallObject::Move(GLfloat dt, GLuint window_width)
	{
		// 如果没有被固定在挡板上
		if (!this->Stuck)
		{
			// 移动球
			this->Position += this->Velocity * dt;
			// 检查是否在窗口边界以外，如果是的话反转速度并恢复到正确的位置
			if (this->Position.x <= 0.0f)
			{
				this->Velocity.x = -this->Velocity.x;
				this->Position.x = 0.0f;
			}
			else if (this->Position.x + this->Size.x >= window_width)
			{
				this->Velocity.x = -this->Velocity.x;
				this->Position.x = window_width - this->Size.x;
			}
			if (this->Position.y <= 0.0f)
			{
				this->Velocity.y = -this->Velocity.y;
				this->Position.y = 0.0f;
			}

		}
		return this->Position;
	}
	*/
	//除了反转球的速度之外，我们还需要把球沿着边界重新放置回来。
	//只有在没有被固定时球才能够移动。

	//因为如果球碰触到底部边界时玩家会结束游戏（或失去一条命），
	//所以在底部边界没有代码来控制球反弹。
	//我们稍后需要在代码中某些地方实现这一逻辑。

	//你可以在下边看到BallObject的代码：
	// ballObject.h

	//首先我们在游戏中添加球。
	//与玩家挡板相似，我们创建一个球对象并且定义两个用来初始化球的常量。
	//对于球的纹理，我们会使用在LearnOpenGL Breakout游戏中完美适用的一张图片：球纹理。
	/*
	// 初始化球的速度
	const glm::vec2 INITIAL_BALL_VELOCITY(100.0f, -350.0f);
	// 球的半径
	const GLfloat BALL_RADIUS = 12.5f;

	BallObject     *Ball;

	void Game::Init()
	{
		[...]
		glm::vec2 ballPos = playerPos + glm::vec2(PLAYER_SIZE.x / 2 - BALL_RADIUS, -BALL_RADIUS * 2);
		Ball = new BallObject(ballPos, BALL_RADIUS, INITIAL_BALL_VELOCITY,
			ResourceManager::GetTexture("face"));
	}
	*/

	//然后我们在每帧中调用游戏代码中Update函数里的Move函数来更新球的位置：
	/*
	void Game::Update(GLfloat dt)
	{
		Ball->Move(dt, this->Width);
	}
	*/
	int AH;
	//除此之外，由于球初始是固定在挡板上的，我们必须让玩家能够从固定的位置重新移动它。
	//我们选择使用空格键来从挡板释放球。这意味着我们必须稍微修改ProcessInput函数：
	/*
	void Game::ProcessInput(GLfloat dt)
	{
		if (this->State == GAME_ACTIVE)
		{
			GLfloat velocity = PLAYER_VELOCITY * dt;
			// 移动玩家挡板
			if (this->Keys[GLFW_KEY_A])
			{
				if (Player->Position.x >= 0)
				{
					Player->Position.x -= velocity;
					if (Ball->Stuck)
						Ball->Position.x -= velocity;
				}
			}
			if (this->Keys[GLFW_KEY_D])
			{
				if (Player->Position.x <= this->Width - Player->Size.x)
				{
					Player->Position.x += velocity;
					if (Ball->Stuck)
						Ball->Position.x += velocity;
				}
			}
			if (this->Keys[GLFW_KEY_SPACE])
				Ball->Stuck = false;
		}
	}
	*/

	//现在如果玩家按下了空格键，球的Stuck值会设置为false。
	//我们还需要更新ProcessInput函数，当球被固定的时候，会跟随挡板的位置来移动球。

	//最后我们需要渲染球，此时这应该很显而易见了：
	/*
	void Game::Render()
	{
		if (this->State == GAME_ACTIVE)
		{
			[...]
			Ball->Draw(*Renderer);
		}
	}
	*/

	//结果就是球会跟随着挡板，并且当我们按下空格键时球开始自由运动。
	//球会在左侧、右侧和顶部边界合理地反弹，但看起来不会撞击任何的砖块，
	//就像我们可以在下边的视频中看到的那样：
	// https://learnopengl-cn.github.io/img/06/Breakout/05/01/no_collisions.mp4

	//我们要做的就是创建一个或多个函数用于检查球对象是否撞击关卡中的任何砖块，如果撞击的话就销毁砖块。
	//这些所谓的碰撞检测(collision detection)功能将是我们下一个教程的重点。

}

//碰撞-碰撞检测
namespace CollisionDetection
{
	//当试图判断两个物体之间是否有碰撞发生时，
	//我们通常不使用物体本身的数据，因为这些物体常常会很复杂，
	//这将导致碰撞检测变得很复杂。
	//正因这一点，使用重叠在物体上的更简单的外形（通常有较简单明确的数学定义）来进行碰撞检测成为常用的方法。
	//我们基于这些简单的外形来检测碰撞，这样代码会变得更简单且节省了很多性能。
	//这些碰撞外形例如圆、球体、长方形和立方体等，
	//与拥有上百个三角形的网格相比简单了很多。

	//虽然它们确实提供了更简单更高效的碰撞检测算法，
	//但这些简单的碰撞外形拥有一个共同的缺点，
	//这些外形通常无法完全包裹物体。
	//产生的影响就是当检测到碰撞时，实际的物体并没有真正的碰撞。
	//必须记住的是这些外形仅仅是真实外形的近似。

	//AABB - AABB 碰撞-----------------------------------------

	//AABB代表的是轴对齐碰撞箱(Axis-aligned Bounding Box)，
	//碰撞箱是指与场景基础坐标轴（2D中的是x和y轴）对齐的长方形的碰撞外形。
	//与坐标轴对齐意味着这个长方形没有经过旋转并且它的边线和场景中基础坐标轴平行
	//（例如，左右边线和y轴平行）。这些碰撞箱总是和场景的坐标轴平行，
	//这使得所有的计算都变得更简单。下边是我们用一个AABB包裹一个球对象（物体）：
	// https://learnopengl-cn.github.io/img/06/Breakout/05/02/collisions_ball_aabb.png

	//Breakout中几乎所有的物体都是基于长方形的物体，
	//因此很理所应当地使用轴对齐碰撞箱来进行碰撞检测。这就是我们接下来要做的。

	//有多种方式来定义与坐标轴对齐的碰撞箱。
	//其中一种定义AABB的方式是获取左上角点和右下角点的位置。
	//我们定义的GameObject类已经包含了一个左上角点位置（它的Position vector）
	//并且我们可以通过把左上角点的矢量加上它的尺寸（Position + Size）很容易地计算出右下角点。
	//每个GameObject都包含一个AABB我们可以高效地使用它们碰撞。

	//那么我们如何判断碰撞呢？
	//当两个碰撞体积进入对方的区域时就会发生碰撞，
	//例如定义了第一个物体的碰撞体积以某种形式进入了第二个物体的碰撞体积。
	//对于AABB来说很容易判断，因为它们是与坐标轴对齐的：
	//对于每个轴我们要检测两个物体的边界在此轴向是否有重叠。
	//因此我们只是简单地检查两个物体的水平边界是否重合以及垂直边界是否重合。
	//如果水平边界和垂直边界都有重叠那么我们就检测到一次碰撞。
	// https://learnopengl-cn.github.io/img/06/Breakout/05/02/collisions_overlap.png

	//将这一概念转化为代码也是很直白的。
	//我们对两个轴都检测是否重叠，如果都重叠就返回碰撞：
	/*
	GLboolean CheckCollision(GameObject &one, GameObject &two) // AABB - AABB collision
	{
		// x轴方向碰撞？
		bool collisionX = one.Position.x + one.Size.x >= two.Position.x &&
			two.Position.x + two.Size.x >= one.Position.x;
		// y轴方向碰撞？
		bool collisionY = one.Position.y + one.Size.y >= two.Position.y &&
			two.Position.y + two.Size.y >= one.Position.y;
		// 只有两个轴向都有碰撞时才碰撞
		return collisionX && collisionY;
	}
	*/

	//我们检查第一个物体的最右侧是否大于第二个物体的最左侧
	//并且第二个物体的最右侧是否大于第一个物体的最左侧；
	//垂直的轴向与此相似。
	//如果您无法顺利地将这一过程可视化，可以尝试在纸上画边界线/长方形来自行判断。

	//为更好地组织碰撞的代码，我们在Game类中加入一个额外的函数：
	/*
	class Game
	{
		public:
			[...]
			void DoCollisions();
	};
	*/

	//我们可以使用DoCollisions来检查球与关卡中的砖块是否发生碰撞。
	//如果检测到碰撞，就将砖块的Destroyed属性设为true，此举会停止关卡中对此砖块的渲染。
	/*
	void Game::DoCollisions()
	{
		for (GameObject &box : this->Levels[this->Level].Bricks)
		{
			if (!box.Destroyed)
			{
				if (CheckCollision(*Ball, box))
				{
					if (!box.IsSolid)
						box.Destroyed = GL_TRUE;
				}
			}
		}
	}
	*/

	//接下来我们需要更新Game的Update函数：
	/*
	void Game::Update(GLfloat dt)
	{
		// 更新对象
		Ball->Move(dt, this->Width);
		// 检测碰撞
		this->DoCollisions();
	}
	*/
	int next;

	//此时如果我们运行代码，球会与每个砖块进行碰撞检测，
	//如果砖块不是实心的，则表示砖块被销毁。如果运行游戏以下是你会看到的：
	// https://learnopengl-cn.github.io/img/06/Breakout/05/02/collisions.mp4
	//虽然碰撞检测确实生效，但并不是非常准确，因为球会在不直接接触到大多数砖块时与它们发生碰撞。
	//我们来实现另一种碰撞检测技术。

	//AABB - 圆碰撞检测------------------------------------------------------------------

	//由于球是一个圆形的物体，AABB或许不是球的最佳碰撞外形。
	//碰撞的代码中将球视为长方形框，
	//因此常常会出现球碰撞了砖块但此时球精灵还没有接触到砖块。
	// https://learnopengl-cn.github.io/img/06/Breakout/05/02/collisions_ball_aabb_touch.png

	//使用圆形碰撞外形而不是AABB来代表球会更合乎常理。
	//因此我们在球对象中包含了Radius变量，为了定义圆形碰撞外形，
	//我们需要的是一个位置矢量和一个半径。
	// https://learnopengl-cn.github.io/img/06/Breakout/05/02/collisions_circle.png

	//这意味着我们不得不修改检测算法，因为当前的算法只适用于两个AABB的碰撞。
	//检测圆和AABB碰撞的算法会稍稍复杂，关键点如下：
	//我们会找到AABB上距离圆最近的一个点，
	//如果圆到这一点的距离小于它的半径，那么就产生了碰撞。

	//难点在于获取AABB上的最近点P¯。下图展示了对于任意的AABB和圆我们如何计算该点：
	// https://learnopengl-cn.github.io/img/06/Breakout/05/02/collisions_aabb_circle.png

	//首先我们要获取球心C¯与AABB中心B¯的矢量差D¯。
	//接下来用AABB的半边长(half - extents)w和h¯来限制(clamp)矢量D¯。
	//长方形的半边长是指长方形的中心到它的边的距离；
	//简单的说就是它的尺寸除以2。
	//这一过程返回的是一个总是位于AABB的边上的位置矢量（除非圆心在AABB内部）。

	//限制运算把一个值限制在给定范围内，并返回限制后的值。
	//通常可以表示为：
	float clamp(float value, float min, float max)
	{
		return std::max(min, std::min(max, value));
	}
	//例如，值42.0f被限制到6.0f和3.0f之间会得到6.0f；而4.20f会被限制为4.20f。
	//限制一个2D的矢量表示将其x和y分量都限制在给定的范围内。

	//这个限制后矢量P¯就是AABB上距离圆最近的点。
	//接下来我们需要做的就是计算一个新的差矢量D¯，它是圆心C¯和P¯的差矢量。
	// https://learnopengl-cn.github.io/img/06/Breakout/05/02/collisions_aabb_circle_radius_compare.png

	//既然我们已经有了矢量D¯，我们就可以比较它的长度和圆的半径以判断是否发生了碰撞。
	//这一过程通过下边的代码来表示：
	GLboolean CheckCollision(BallObject& one, GameObject& two) // AABB - Circle collision
	{
		// 获取圆的中心 
		glm::vec2 center(one.Position + one.Radius);
		// 计算AABB的信息（中心、半边长）
		glm::vec2 aabb_half_extents(two.Size.x / 2, two.Size.y / 2);
		glm::vec2 aabb_center(
			two.Position.x + aabb_half_extents.x,
			two.Position.y + aabb_half_extents.y
		);
		// 获取两个中心的差矢量
		glm::vec2 difference = center - aabb_center;
		glm::vec2 clamped = glm::clamp(difference, -aabb_half_extents, aabb_half_extents);
		// AABB_center加上clamped这样就得到了碰撞箱上距离圆最近的点closest
		glm::vec2 closest = aabb_center + clamped;
		// 获得圆心center和最近点closest的矢量并判断是否 length <= radius
		difference = closest - center;
		return glm::length(difference) < one.Radius;
	}

	//我们创建了CheckCollision的一个重载函数用于专门处理一个BallObject和一个GameObject的情况。
	//因为我们并没有在对象中保存碰撞外形的信息，因此我们必须为其计算：
	//首先计算球心，然后是AABB的半边长及中心。

	//使用这些碰撞外形的参数，我们计算出differenceD¯然后得到限制后的值clamped，
	//并与AABB中心相加得到closestP¯。
	//然后计算出center和closest的矢量差D¯并返回两个外形是否碰撞。

	//之前我们调用CheckCollision时将球对象作为其第一个参数，
	//因此现在CheckCollision的重载变量会自动生效，我们无需修改任何代码。
	//现在的结果会比之前的碰撞检测算法更准确。

	// https://learnopengl-cn.github.io/img/06/Breakout/05/02/collisions_circle.mp4
	//看起来生效了，但仍缺少一些东西。
	//我们准确地检测了所有碰撞，但碰撞并没有对球产生任何反作用。
	//我们需要在碰撞时产生一些反作用，例如当碰撞发生时，更新球的位置和/或速度。
	//这将是下一个教程的主题。
}

//碰撞-碰撞处理
namespace CollisionResolution
{
	//上个教程的最后，我们得到了一种有效的碰撞检测方案。
	//但是球对检测到的碰撞不会有反作用；它仅仅是径直穿过所有的砖块。
	//我们希望球会从撞击到的砖块反弹。
	//此教程将讨论如何使用AABB-圆碰撞方案实现这项称为碰撞处理(Collision Resolution)的功能。

	//当碰撞发生时，我们希望出现两个现象：
	//重新定位球，以免它进入另一个物体，
	//其次是改变球的速度方向，使它看起来像是物体的反弹。

	//碰撞重定位----------------------------------------------------------------------

	//为了把球对象定位到碰撞的AABB的外部，我们必须明确球侵入碰撞框的距离。
	//为此我们要回顾上一节教程中的示意图：
	// https://learnopengl-cn.github.io/img/06/Breakout/05/03/collisions_aabb_circle_resolution.png

	//此时球少量进入了AABB，所以检测到了碰撞。
	//我们现在希望将球从移出AABB的外形使其仅仅碰触到AABB，像是没有碰撞一样。
	//为了确定需要将球从AABB中移出多少距离，我们需要找回矢量R¯，它代表的是侵入AABB的程度。
	//为得到R¯我们用球的半径减去V¯。
	//矢量V¯是最近点P¯和球心C¯的差矢量。

	//有了R¯之后我们将球的位置偏移R¯就将球直接放置在与AABB紧邻的位置；
	//此时球已经被重定位到合适的位置。

	//碰撞方向------------------------------------------------------------------------

	//下一步我们需要确定碰撞之后如何更新球的速度。
	//对于Breakout我们使用以下规则来改变球的速度：
	// 如果球撞击AABB的右侧或左侧，它的水平速度（x）将会反转。
	// 如果球撞击AABB的上侧或下侧，它的垂直速度（y）将会反转。

	//但是如何判断球撞击AABB的方向呢？解决这一问题有很多种方法，
	//其中之一是对于每个砖块使用4个AABB而不是1个AABB，并把它们放置到砖块的每个边上。
	//使用这种方法我们可以确定被碰撞的是哪个AABB和哪个边。
	//但是有一种使用点乘(dot product)的更简单的方法。

	//您或许还记得变换教程中点乘可以得到两个正交化的矢量的夹角。
	//如果我们定义指向北、南、西和东的四个矢量，
	//然后计算它们和给定矢量的夹角会怎么样？
	//由这四个方向矢量和给定的矢量点乘积的结果中的最高值
	//（点乘积的最大值为1.0f，代表0度角）即是矢量的方向。

	//这一过程如下代码所示：
	/*
	Direction VectorDirection(glm::vec2 target)
	{
		glm::vec2 compass[] = {
			glm::vec2(0.0f, 1.0f),  // 上
			glm::vec2(1.0f, 0.0f),  // 右
			glm::vec2(0.0f, -1.0f), // 下
			glm::vec2(-1.0f, 0.0f)  // 左
		};
		GLfloat max = 0.0f;
		GLuint best_match = -1;
		for (GLuint i = 0; i < 4; i++)
		{
			GLfloat dot_product = glm::dot(glm::normalize(target), compass[i]);
			if (dot_product > max)
			{
				max = dot_product;
				best_match = i;
			}
		}
		return (Direction)best_match;
	}
	*/

	//此函数比较了target矢量和compass数组中各方向矢量。
	//compass数组中与target角度最接近的矢量，即是返回给函数调用者的Direction。
	//这里的Direction是一个Game类的头文件中定义的枚举类型：
	enum Direction
	{
		UP,
		RIGHT,
		DOWN,
		LEFT
	};

	//既然我们已经知道了如何获得R¯以及如何判断球撞击AABB的方向，
	//我们开始编写碰撞处理的代码。

	//AABB - 圆碰撞检测----------------------------------------------------------------

	//为了计算碰撞处理所需的数值我们要从碰撞的函数中获取更多的信息而不只只是一个true或false，
	//因此我们要返回一个包含更多信息的tuple，这些信息即是碰撞发生时的方向及差矢量（R¯）。
	//你可以在头文件tuple中找到tuple。

	//为了更好组织代码，我们把碰撞相关的数据使用typedef定义为Collision：
	typedef std::tuple<GLboolean, Direction, glm::vec2> Collision;

	//接下来我们还需要修改CheckCollision函数的代码，
	//使其不仅仅返回true或false,还包含方向和差矢量：
	/*
	Collision CheckCollision(BallObject& one, GameObject& two) // AABB - AABB 碰撞
	{
		[...]
			if (glm::length(difference) <= one.Radius)
				return std::make_tuple(GL_TRUE, VectorDirection(difference), difference);
			else
				return std::make_tuple(GL_FALSE, UP, glm::vec2(0, 0));
	}
	*/

	//Game类的DoCollision函数现在不仅仅只检测是否出现了碰撞，
	//而且在碰撞发生时会有适当的动作。
	//此函数现在会计算碰撞侵入的程度（如本教程一开始计时的示意图中所示）
	//并且基于碰撞方向使球的位置矢量与其相加或相减。
	/*
	void Game::DoCollisions()
	{
		for (GameObject &box : this->Levels[this->Level].Bricks)
		{
			if (!box.Destroyed)
			{
				Collision collision = CheckCollision(*Ball, box);
				if (std::get<0>(collision)) // 如果collision 是 true
				{
					// 如果砖块不是实心就销毁砖块
					if (!box.IsSolid)
						box.Destroyed = GL_TRUE;
					// 碰撞处理
					Direction dir = std::get<1>(collision);
					glm::vec2 diff_vector = std::get<2>(collision);
					if (dir == LEFT || dir == RIGHT) // 水平方向碰撞
					{
						Ball->Velocity.x = -Ball->Velocity.x; // 反转水平速度
						// 重定位
						GLfloat penetration = Ball->Radius - std::abs(diff_vector.x);
						if (dir == LEFT)
							Ball->Position.x += penetration; // 将球右移
						else
							Ball->Position.x -= penetration; // 将球左移
					}
					else // 垂直方向碰撞
					{
						Ball->Velocity.y = -Ball->Velocity.y; // 反转垂直速度
						// 重定位
						GLfloat penetration = Ball->Radius - std::abs(diff_vector.y);
						if (dir == UP)
							Ball->Position.y -= penetration; // 将球上移
						else
							Ball->Position.y += penetration; // 将球下移
					}
				}
			}
		}
	}
	*/

	//不要被函数的复杂度给吓到，因为它仅仅是我们目前为止的概念的直接转化。
	//首先我们会检测碰撞如果发生了碰撞且砖块不是实心的那么就销毁砖块。
	//然后我们从tuple中获取到了碰撞的方向dir以及表示V¯的差矢量diff_vector，
	//最终完成碰撞处理。

	//我们首先检查碰撞方向是水平还是垂直，并据此反转速度。
	//如果是水平方向，我们从diff_vector的x分量计算侵入量RR并根据碰撞方向用球的位置矢量加上或减去它。
	//垂直方向的碰撞也是如此，但是我们要操作各矢量的y分量。

	//现在运行你的应用程序，应该会向你展示一套奏效的碰撞方案，
	//但可能会很难真正看到它的效果，因为一旦球碰撞到了一个砖块就会弹向底部并永远丢失。
	//我们可以通过处理玩家挡板的碰撞来修复这一问题。

	//玩家 - 球碰撞-------------------------------------------------------------------

	//球和玩家之间的碰撞与我们之前讨论的碰撞稍有不同，
	//因为这里应当基于撞击挡板的点与（挡板）中心的距离来改变球的水平速度。
	//撞击点距离挡板的中心点越远，则水平方向的速度就会越大。
	/*
	void Game::DoCollisions()
	{
		[...]
		Collision result = CheckCollision(*Ball, *Player);
		if (!Ball->Stuck && std::get<0>(result))
		{
			// 检查碰到了挡板的哪个位置，并根据碰到哪个位置来改变速度
			GLfloat centerBoard = Player->Position.x + Player->Size.x / 2;
			GLfloat distance = (Ball->Position.x + Ball->Radius) - centerBoard;
			GLfloat percentage = distance / (Player->Size.x / 2);
			// 依据结果移动
			GLfloat strength = 2.0f;
			glm::vec2 oldVelocity = Ball->Velocity;
			Ball->Velocity.x = INITIAL_BALL_VELOCITY.x * percentage * strength;
			Ball->Velocity.y = -Ball->Velocity.y;
			Ball->Velocity = glm::normalize(Ball->Velocity) * glm::length(oldVelocity);
		}
	}
	*/

	//在我们完成了球和各砖块的碰撞检测之后，我们来检测球和玩家挡板是否发生碰撞。
	//如果有碰撞（并且球不是被固定在挡板上）我们要计算球的中心与挡板中心的距离和挡板的半边长的百分比。
	//之后球的水平速度会依据它撞击挡板的点到挡板中心的距离来更新。
	//除了更新水平速度之外我们还需要反转它的y方向速度。
	int goon;

	//注意旧的速度被存储为oldVelocity。
	//之所以要存储旧的速度是因为我们只更新球的速度矢量中水平方向的速度并保持它的y速度不变。
	//这将意味着矢量的长度会持续变化，
	//其产生的影响是如果球撞击到挡板的边缘则会比撞击到挡板中心有更大(也因此更强)的速度矢量。
	//为此新的速度矢量会正交化然后乘以旧速度矢量的长度。
	//这样一来，球的力量和速度将总是一一致的，无论它撞击到挡板的哪个地方。

	//粘板-------------------------------------------------------------------------------

	//无论你有没有注意到，但当运行代码时，球和玩家挡板的碰撞处理仍旧有一个大问题。
	//以下的视频清楚地展示了将会出现的现象：https://learnopengl-cn.github.io/img/06/Breakout/05/03/collisions_sticky_paddle.mp4

	//这种问题称为粘板问题(Sticky Paddle Issue)，出现的原因是玩家挡板以较高的速度移向球，导致球的中心进入玩家挡板。
	//由于我们没有考虑球的中心在AABB内部的情况，游戏会持续试图对所有的碰撞做出响应，
	//当球最终脱离时，已经对y向速度翻转了多次，以至于无法确定球在脱离后是向上还是向下运动。

	//我们可以引入一个小的特殊处理来很容易地修复这种行为，
	//这个处理之所以成为可能是基于我们可以假设碰撞总是发生在挡板顶部的事实。
	//我们总是简单地返回正的y速度而不是反转y速度，这样当它被卡住时也可以立即脱离。
	/*
	 //Ball->Velocity.y = -Ball->Velocity.y;
	Ball->Velocity.y = -1 * abs(Ball->Velocity.y);
	*/

	//如果你足够仔细就会觉得这一影响仍然是可以被注意到的，
	//但是我个人将此方法当作一种可接受的折衷处理。

	//底部边界-----------------------------------------------------------------------

	//与经典的Breakout内容相比唯一缺少的就是失败条件了，失败会重置关卡和玩家。
	//在Game类的Update函数中，我们要检查球是否接触到了底部边界，如果接触到就重置游戏。
	/*
	void Game::Update(GLfloat dt)
	{
		[...]
		if (Ball->Position.y >= this->Height) // 球是否接触底部边界？
		{
			this->ResetLevel();
			this->ResetPlayer();
		}
	}
	*/

	//ResetLevel和ResetPlayer函数直接重新加载关卡并重置对象的各变量值为原始的值。
	//现在游戏看起来应该是这样的：https://learnopengl-cn.github.io/img/06/Breakout/05/03/collisions_complete.mp4

	//就是这样了，我们创建完成了一个有相似机制的经典Breakout游戏的复制版。

	//注意事项------------------------------------------------------------------------

	//在视频游戏的发展过程中，碰撞检测是一个困难的话题甚至可能是最大的挑战。
	//大多数的碰撞检测和处理方案是和物理引擎合并在一起的，
	//正如多数现代的游戏中看到的那样。
	//我们在Breakout游戏中使用的碰撞方案是一个非常简单的方案并且是专门给这类游戏所专用的。

	//需要强调的是这类碰撞检测和处理方式是不完美的。
	//它只能计算每帧内可能发生的碰撞并且只能计算在该时间步时物体所在的各位置；
	//这意味着如果一个物体拥有一个很大的速度以致于在一帧内穿过了另一个物体，
	//它将看起来像是从来没有与另一个物体碰撞过。
	//因此如果出现掉帧或出现了足够高的速度，这一碰撞检测方案将无法应对。

	//（我们使用的碰撞方案）仍然会出现这几个问题：

	//*如果球运动得足够快，它可能在一帧内完整地穿过一个物体，而不会检测到碰撞。
	//*如果球在一帧内同时撞击了一个以上的物体，它将会检测到两次碰撞并两次反转速度；
	// 这样不改变它的原始速度。
	//*撞击到砖块的角时会在错误的方向反转速度，
	// 这是因为它在一帧内穿过的距离会引发VectorDirection返回水平方向还是垂直方向的差别。

	//但是，本教程目的在于教会读者们图形学和游戏开发的基础知识。
	//因此，这里的碰撞方案可以服务于此目的；
	//它更容易理解且在正常的场景中可以较好地运作。
	//需要记住的是存在有更好的（更复杂）碰撞方案，
	//在几乎所有的场景中都可以很好地运作（包括可移动的物体）如分离轴定理(Separating Axis Theorem)。

	//值得庆幸的是，有大量实用并且常常很高效的物理引擎（使用时间步无关的碰撞方案）可供您在游戏中使用。
	//如果您希望在这一系统中有更深入的探索或需要更高级的物理系统又不理解其中的数学机理，
	//Box2D是一个实现了物理系统和碰撞检测的可以用在您的应用程序中的完美的2D物理库。
	// http://box2d.org/
}

//粒子
namespace aParticles
{
	GameObject object;
	glm::vec2 offset;
	GLfloat dt;

	//一个微粒,从OpenGL的角度看就是一个总是面向摄像机方向且(通常)包含一个大部分区域是透明的纹理的小四边形。
	//一个微粒本身主要就是一个精灵(sprite),前面我们已经早就使用过了，
	//但是当你把成千上万个这些微粒放在一起的时候,就可以创造出令人疯狂的效果.

	//当处理这些微粒的时候，通常是由一个叫做粒子发射器或粒子生成器的东西完成的，
	//从这个地方,持续不断的产生新的微粒并且旧的微粒随着时间逐渐消亡。
	//如果这个粒子发射器产生一个带着类似烟雾纹理的微粒的时候，
	//它的颜色亮度同时又随着与发射器距离的增加而变暗，那么就会产生出灼热的火焰的效果:
	// https://learnopengl-cn.github.io/img/06/Breakout/06/particles_example.jpg

	//一个单一的微粒通常有一个生命值变量，并且从它产生开始就一直在缓慢的减少。
	//一旦它的生命值少于某个极限值（通常是0）我们就会杀掉这个粒子,
	//这样下一个粒子产生时就可以让它来替换那个被杀掉的粒子。
	//一个粒子发射器控制它产生的所有粒子并且根据它们的属性来改变它们的行为。
	//一个粒子通常有下面的属性：
	struct Particle
	{
		glm::vec2 Position, Velocity;
		glm::vec4 Color;
		GLfloat Life;

		Particle()
			: Position(0.0f), Velocity(0.0f), Color(1.0f), Life(0.0f) { }
	};

	//看上面那个火焰的例子，那个粒子发射器可能在靠近发射器的地方产生每一个粒子，
	//并且有一个向上的速度，这样每个粒子都是朝着正$y$轴方向移动。
	//那似乎有3个不同区域，只是可能相比其他的区域，给了某个区域内的粒子更快的速度。
	//我们也可以看到，y轴方向越高的粒子，它们的黄色或者说亮度就越低。
	//一旦某个粒子到达某个高度的时候，它的生命值就会耗尽然后被杀掉；绝不可能直冲云霄。

	//你可以想象到用这样一个系统，我们就可以创造一些有趣的效果比如火焰，青烟，烟雾，魔法效果，炮火残渣等等。
	//在Breakout游戏里，我们将会使用下面那个小球来创建一个简单的粒子生成器来制作一些有趣的效果，
	//结果看起来就像这样：https://learnopengl-cn.github.io/img/06/Breakout/06/particles.mp4

	//上面那个粒子生成器在这个球的位置产生无数的粒子，根据球移动的速度给了粒子相应的速度，
	//并且根据它们的生命值来改变他们的颜色亮度。

	//为了渲染这些粒子，我们将会用到有不同实现的着色器：
	/*
	#version 330 core
	layout (location = 0) in vec4 vertex; // <vec2 position, vec2 texCoords>

	out vec2 TexCoords;
	out vec4 ParticleColor;

	uniform mat4 projection;
	uniform vec2 offset;
	uniform vec4 color;

	void main()
	{
		float scale = 10.0f;
		TexCoords = vertex.zw;
		ParticleColor = color;
		gl_Position = projection * vec4((vertex.xy * scale) + offset, 0.0, 1.0);
	}
	*/

	//以及像素着色器:
	/*
	#version 330 core
	in vec2 TexCoords;
	in vec4 ParticleColor;
	out vec4 color;

	uniform sampler2D sprite;

	void main()
	{
		color = (texture(sprite, TexCoords) * ParticleColor);
	}
	*/

	//我们获取每个粒子的位置和纹理属性并且设置两个uniform变量：offset和color来改变每个粒子的输出状态。
	//注意到，在顶点着色器里，我们把这个四边形的粒子缩小了10倍；
	//你也可以把这个缩放变量设置成uniform类型的变量从而控制一些个别的粒子。

	//首先,我们需要一个粒子数组，然后用Particle结构体的默认构造函数来实例化。
	GLuint nr_particles = 500;
	std::vector<Particle> particles;

	void how()
	{
		for (GLuint i = 0; i < nr_particles; ++i)
			particles.push_back(Particle());

		//然后在每一帧里面，我们都会用一个起始变量来产生一些新的粒子并且对每个粒子（还活着的）更新它们的值。
		GLuint nr_new_particles = 2;
		// Add new particles
		for (GLuint i = 0; i < nr_new_particles; ++i)
		{
			int unusedParticle = FirstUnusedParticle();
			RespawnParticle(particles[unusedParticle], object, offset);
		}
		// Update all particles
		for (GLuint i = 0; i < nr_particles; ++i)
		{
			Particle& p = particles[i];
			p.Life -= dt; // reduce life
			if (p.Life > 0.0f)
			{   // particle is alive, thus update
				p.Position -= p.Velocity * dt;
				p.Color.a -= dt * 2.5;
			}
		}

		//第一个循环看起来可能有点吓人。
		//因为这些粒子会随着时间消亡，我们就想在每一帧里面产生nr_new_particles个新粒子。
		//但是一开始我们就知道了总的粒子数量是nr_partiles，所以我们不能简单的往粒子数组里面添加新的粒子。
		//否则的话我们很快就会得到一个装满成千上万个粒子的数组，
		//考虑到这个粒子数组里面其实只有一小部分粒子是存活的，这样就太浪费效率了。

		//（补充）实际上这里还有一个地方在浪费资源：透明度将为0但仍未死亡的粒子
		//如果将判断透明度<=0则直接令粒子死亡，则可以将粒子容量从500降到200（这个例子中，其实透明度就可作为生命值）
	}
	//我们要做的就是找到第一个消亡的粒子然后用一个新产生的粒子来更新它。
	//函数FirstUnuseParticle就是试图找到第一个消亡的粒子并且返回它的索引值给调用者。
	GLuint lastUsedParticle = 0;
	GLuint FirstUnusedParticle()
	{
		// Search from last used particle, this will usually return almost instantly
		for (GLuint i = lastUsedParticle; i < nr_particles; ++i) {
			if (particles[i].Life <= 0.0f) {
				lastUsedParticle = i;
				return i;
			}
		}
		// Otherwise, do a linear search
		for (GLuint i = 0; i < lastUsedParticle; ++i) {
			if (particles[i].Life <= 0.0f) {
				lastUsedParticle = i;
				return i;
			}
		}
		// Override first particle if all others are alive
		lastUsedParticle = 0;
		return 0;
	}

	//这个函数存储了它找到的上一个消亡的粒子的索引值，
	//由于下一个消亡的粒子索引值总是在上一个消亡的粒子索引值的右边，
	//所以我们首先从它存储的上一个消亡的粒子索引位置开始查找，
	//如果我们没有任何消亡的粒子，我们就简单的做一个线性查找，
	//如果没有粒子消亡就返回索引值0，结果就是第一个粒子被覆盖，
	//需要注意的是，如果是最后一种情况，就意味着你粒子的生命值太长了，
	//在每一帧里面需要产生更少的粒子，或者你只是没有保留足够的粒子。

	//之后，一旦粒子数组中第一个消亡的粒子被发现的时候，
	//我们就通过调用RespawnParticle函数更新它的值，
	//函数接受一个Particle对象，一个GameObject对象和一个offset向量:
	void RespawnParticle(Particle& particle, GameObject& object, glm::vec2 offset)
	{
		GLfloat random = ((rand() % 100) - 50) / 10.0f;
		GLfloat rColor = 0.5 + ((rand() % 100) / 100.0f);
		particle.Position = object.Position + random + offset;
		particle.Color = glm::vec4(rColor, rColor, rColor, 1.0f);
		particle.Life = 1.0f;
		particle.Velocity = object.Velocity * 0.1f;
	}
	//这个函数简单的重置这个粒子的生命值为1.0f，
	//随机的给一个大于0.5的颜色值(经过颜色向量)并且(在物体周围)分配一个位置和速度基于游戏里的物体。

	//对于更新函数里的第二个循环遍历了所有粒子，并且对于每个粒子的生命值都减去一个时间差；
	//这样每个粒子的生命值就精确到了秒。
	//然后再检查这个粒子是否是还活着的，若是,则更新它的位置和颜色属性。
	//这里我们缓慢的减少粒子颜色值的alpha值，以至于它看起来就是随着时间而缓慢的消亡。

	//最后保留下来就是实际需要渲染的粒子：
	/*
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	particleShader.Use();
	for (Particle particle : particles)
	{
		if (particle.Life > 0.0f)
		{
			particleShader.SetVector2f("offset", particle.Position);
			particleShader.SetVector4f("color", particle.Color);
			particleTexture.Bind();
			glBindVertexArray(particleVAO);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			glBindVertexArray(0);
		}
	}
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	*/

	//在这，对于每个粒子，我们一一设置他们的uniform变量offse和color，绑定纹理，然后渲染2D四边形的粒子。
	//有趣的是我们在这看到了两次调用函数glBlendFunc。
	//当要渲染这些粒子的时候，我们使用GL_ONE替换默认的目的因子模式GL_ONE_MINUS_SRC_ALPHA，
	//这样，这些粒子叠加在一起的时候就会产生一些平滑的发热效果，
	//就像在这个教程前面那样使用混合模式来渲染出火焰的效果也是可以的，
	//这样在有大多数粒子的中心就会产生更加灼热的效果。

	//因为我们(就像这个系列教程的其他部分一样)喜欢让事情变得有条理，
	//所以我们就创建了另一个类ParticleGenerator来封装我们刚刚谈到的所有功能。
	//你可以在下面的链接里找到源码：
	//particle.h

	//然后在游戏代码里,我们创建这样一个粒子发射器并且用这个纹理初始化。https://learnopengl-cn.github.io/img/06/Breakout/06/particle.png
	/*
	ParticleGenerator   *Particles;

	void Game::Init()
	{
		[...]
		ResourceManager::LoadShader("shaders/particle.vs", "shaders/particle.frag", nullptr, "particle");
		[...]
		ResourceManager::LoadTexture("textures/particle.png", GL_TRUE, "particle");
		[...]
		Particles = new ParticleGenerator(
			ResourceManager::GetShader("particle"),
			ResourceManager::GetTexture("particle"),
			500
		);
	}
	*/

	//然后我们在Game类的Updata函数里为粒子生成器添加一条更新语句：
	/*
	void Game::Update(GLfloat dt)
	{
		[...]
		// Update particles
		Particles->Update(dt, *Ball, 2, glm::vec2(Ball->Radius / 2));
		[...]
	}
	*/

	//每个粒子都将使用球的游戏对象属性对象，
	//每帧产生两个粒子并且他们都是偏向球得中心，最后是渲染粒子：
	/*
	void Game::Render()
	{
		if (this->State == GAME_ACTIVE)
		{
			[...]
			// Draw player
			Player->Draw(*Renderer);
			// Draw particles
			Particles->Draw();
			// Draw ball
			Ball->Draw(*Renderer);
		}
	}
	*/

	//注意到，我们是在渲染球体之前且在渲染其他物体之后渲染粒子的，
	//这样，粒子就会在所有其他物体面前。

	//如果你现在编译并运行你的程序，你可能会看到在球体之后有一条小尾巴。
	//就像这个教程开始的那样，给了这个游戏更加现代化的面貌。
	//这个系统还可以很容易的扩展到更高级效果的主体上，
	//就用这个粒子生成器自由的去实验吧，看看你是否可以创建出你自己的特效。
}

//后期处理
namespace Postprocessing
{
	//如果我们可以通过几个后期处理(Postprocess)特效丰富Breakout游戏的视觉效果的话，
	//会不会是一件很有趣的事情？利用OpenGL的帧缓冲，
	//我们可以相对容易地创造出模糊的抖动效果、反转场景里的所有颜色、
	//做一些“疯狂”的顶点运动、或是使用一些其他有趣的特效。

	//这章教程广泛运用了之前帧缓冲与抗锯齿章节的概念。

	//在教程的帧缓冲章节里，我们演示了如何使用单个纹理，
	//通过后期处理特效实现有趣的效果（反相、灰度、模糊、锐化、边缘检测）。
	//在Breakout中我们将做一些类似的事情：
	//我们会创建一个帧缓冲对象，并附带一个多重采样的渲染缓冲对象作为其颜色附件。
	//游戏中所有的渲染相关代码都应该渲染至这个多重采样的帧缓冲，
	//然后将其内容传输(Bit blit)至一个不同的帧缓冲中，该帧缓冲用一个纹理作为其颜色附件。
	//这个纹理会包含游戏的渲染后的抗锯齿图像，
	//我们对它应用零或多个后期处理特效后渲染至一个大的2D四边形。
	//（译注：这段表述的复杂流程与教程帧缓冲章节的内容相似，原文中包含大量易混淆的名词与代词，建议读者先仔细理解帧缓冲章节的内容与流程）。

	//总结一下这些渲染步骤：
	/*
	1.绑定至多重采样的帧缓冲
	2.和往常一样渲染游戏
	3.将多重采样的帧缓冲内容传输至一个普通的帧缓冲中（这个帧缓冲使用了一个纹理作为其颜色缓冲附件）
	4.解除绑定（绑定回默认的帧缓冲）
	5.在后期处理着色器中使用来自普通帧缓冲的颜色缓冲纹理
	6.渲染屏幕大小的四边形作为后期处理着色器的输出
	*/

	//我们的后期处理着色器允许使用三种特效：shake, confuse和chaos。
	/*
	shake：  轻微晃动场景并附加一个微小的模糊效果。
	confuse：反转场景中的颜色并颠倒x轴和y轴。
	chaos:   利用边缘检测卷积核创造有趣的视觉效果，并以圆形旋转动画的形式移动纹理图片，实现“混沌”特效。
	*/
	//以下是这些效果的示例：
	// https://learnopengl-cn.github.io/img/06/Breakout/07/postprocessing_effects.png

	//在2D四边形上操作的顶点着色器如下所示：
	/*
	#version 330 core
	layout (location = 0) in vec4 vertex; // <vec2 position, vec2 texCoords>

	out vec2 TexCoords;

	uniform bool  chaos;
	uniform bool  confuse;
	uniform bool  shake;
	uniform float time;

	void main()
	{
		gl_Position = vec4(vertex.xy, 0.0f, 1.0f);
		vec2 texture = vertex.zw;
		if(chaos)
		{
			float strength = 0.3;
			vec2 pos = vec2(texture.x + sin(time) * strength, texture.y + cos(time) * strength);
			TexCoords = pos;
		}
		else if(confuse)
		{
			TexCoords = vec2(1.0 - texture.x, 1.0 - texture.y);
		}
		else
		{
			TexCoords = texture;
		}
		if (shake)
		{
			float strength = 0.01;
			gl_Position.x += cos(time * 10) * strength;
			gl_Position.y += cos(time * 15) * strength;
		}
	}
	*/

	//基于uniform是否被设置为true，顶点着色器可以执行不同的分支。
	//如果chaos或confuse被设置为true，顶点着色器将操纵纹理坐标来移动场景（以类似圆的方式平移纹理坐标，或反转纹理坐标）。
	//因为我们将纹理环绕方式设置为了GL_REPEAT，所以chaos特效会导致场景在四边形的各个部分重复。
	//除此之外，如果shake被设置为true，顶点位置将发生小幅移动，就像屏幕晃动一样
	//需要注意的是，chaos与confuse不应同时为true，而shake则可以与其他特效一起生效。

	//当任意特效被激活时，除了偏移顶点的位置和纹理坐标，我们也希望创造显著的视觉效果。
	//我们可以在片段着色器中实现这一点：
	/*
	#version 330 core
	in  vec2  TexCoords;
	out vec4  color;

	uniform sampler2D scene;
	uniform vec2      offsets[9];
	uniform int       edge_kernel[9];
	uniform float     blur_kernel[9];

	uniform bool chaos;
	uniform bool confuse;
	uniform bool shake;

	void main()
	{
		color = vec4(0.0f);
		vec3 sample[9];
		// 如果使用卷积矩阵，则对纹理的偏移像素进行采样
		if(chaos || shake)
			for(int i = 0; i < 9; i++)
				sample[i] = vec3(texture(scene, TexCoords.st + offsets[i]));

		// 处理特效
		if(chaos)
		{
			for(int i = 0; i < 9; i++)
				color += vec4(sample[i] * edge_kernel[i], 0.0f);
			color.a = 1.0f;
		}
		else if(confuse)
		{
			color = vec4(1.0 - texture(scene, TexCoords).rgb, 1.0);
		}
		else if(shake)
		{
			for(int i = 0; i < 9; i++)
				color += vec4(sample[i] * blur_kernel[i], 0.0f);
			color.a = 1.0f;
		}
		else
		{
			color =  texture(scene, TexCoords);
		}
	}
	*/
	int how;

	//这个着色器几乎直接构建自帧缓冲教程的片段着色器，并根据被激活的特效类型进行相应的后期处理。
	//这一次，偏移矩阵(offset matrix)和卷积核作为uniform变量，由应用程序中的代码定义。
	//好处是我们只需要设置这些内容一次，而不必在每个片段着色器执行时重新计算这些矩阵。
	//例如，偏移矩阵的配置如下所示：
	/*
	GLfloat offset = 1.0f / 300.0f;
	GLfloat offsets[9][2] =
	{
		{ -offset,  offset  },  // 左上
		{  0.0f,    offset  },  // 中上
		{  offset,  offset  },  // 右上
		{ -offset,  0.0f    },  // 左中
		{  0.0f,    0.0f    },  // 正中
		{  offset,  0.0f    },  // 右中
		{ -offset, -offset  },  // 左下
		{  0.0f,   -offset  },  // 中下
		{  offset, -offset  }   // 右下
	};
	glUniform2fv(glGetUniformLocation(shader.ID, "offsets"), 9, (GLfloat*)offsets);
	*/

	//由于所有管理帧缓冲器的概念已经在之前的教程中有过广泛的讨论，所以这次我不会深入其细节。
	//下面是PostProcessor类的代码，该类负责管理初始化、写入/读取帧缓存以及渲染屏幕四边形。
	//如果你理解了帧缓冲与反锯齿章节的教程，你应该可以理解它的代码。
	//postProcessor.h

	//有趣的是BeginRender和EndRender函数。由于我们必须将整个游戏场景渲染至帧缓冲中，
	//因此我们可以在场景的渲染代码之前和之后分别调用BeginRender和EndRender。
	//接着，这个类将处理幕后的帧缓冲操作。在游戏的渲染函数中使用PostProcessor类如下所示：
	/*
	PostProcessor   *Effects;

	void Game::Render()
	{
		if (this->State == GAME_ACTIVE)
		{
			Effects->BeginRender();
				// 绘制背景
				// 绘制关卡
				// 绘制挡板
				// 绘制粒子
				// 绘制小球
			Effects->EndRender();
			Effects->Render(glfwGetTime());
		}
	}
	*/

	//无论我们需要什么，我们只需要将需要的PostProcessor类中的特效属性设置为true，其效果就可以立即可见。

	//Shake it------------------------------------------------------------------------------------------------------

	//作为这些效果的演示，我们将模拟球击中坚固的混凝土块时的视觉冲击。
	//无论在哪里发生碰撞，只要在短时间内实现晃动(shake)效果，便能增强撞击的冲击感。

	//我们只想允许晃动效果持续一小段时间。我们可以通过声明一个持有晃动效果持续时间的变量ShakeTime来实现这个功能。
	//无论碰撞何时发生，我们将这个变量重置为一个特定的持续时间:
	/*
	GLfloat ShakeTime = 0.0f;

	void Game::DoCollisions()
	{
		for (GameObject &box : this->Levels[this->Level].Bricks)
		{
			if (!box.Destroyed)
			{
				Collision collision = CheckCollision(*Ball, box);
				if (std::get<0>(collision)) // 如果发生了碰撞
				{
					// 如果不是实心的砖块则摧毁
					if (!box.IsSolid)
						box.Destroyed = GL_TRUE;
					else
					{   // 如果是实心的砖块则激活shake特效
						ShakeTime = 0.05f;
						Effects->Shake = true;
					}
					[...]
				}
			}
		}
		[...]
	}
	*/

	//然后在游戏的Update函数中，我们减少ShakeTime变量的值直到其为0.0，并停用shake特效。
	/*
	void Game::Update(GLfloat dt)
	{
		[...]
		if (ShakeTime > 0.0f)
		{
			ShakeTime -= dt;
			if (ShakeTime <= 0.0f)
				Effects->Shake = false;
		}
	}
	*/
	//这样，每当我们碰到一个实心砖块时，屏幕会短暂地抖动与模糊，给玩家一些小球与坚固物体碰撞的视觉反馈。
	// https://learnopengl.com/video/in-practice/breakout/postprocessing_shake.mp4

	//在下一章关于“道具”的教程中我们将带来另外两种的特效的使用。
}

//道具
namespace Powerups
{
	//Breakout已经接近完成了，但我们可以至少再增加一种游戏机制让它变得更酷。“充电”
	//（译注：Powerups，很多游戏中都会用这个单词指代可以提升能力的道具，本文之后也会用道具一词作为其翻译）怎么样？

	//这个想法的含义是，无论一个砖块何时被摧毁，它都有一定几率产生一个道具块。
	//这样的道具快会缓慢降落，而且当它与玩家挡板发生接触时，会发生基于道具类型的有趣效果。
	//例如，某一种道具可以让玩家挡板变长，另一种道具则可以让小球穿过物体。
	//我们还可以添加一些可以给玩家造成负面影响的负面道具。

	//我们可以将道具建模为具有一些额外属性的GameObject，
	//这也是为什么我们定义一个继承自GameObject的PowerUp类并在其中增添了一些额外的成员属性。

	/*
	const glm::vec2 SIZE(60, 20);
	const glm::vec2 VELOCITY(0.0f, 150.0f);

	class PowerUp : public GameObject
	{
	public:
		// 道具类型
		std::string Type;
		GLfloat     Duration;
		GLboolean   Activated;
		// 构造函数
		PowerUp(std::string type, glm::vec3 color, GLfloat duration,
				glm::vec2 position, Texture2D texture)
			: GameObject(position, SIZE, texture, color, VELOCITY),
			  Type(type), Duration(duration), Activated()
		{ }
	};
	*/

	//每个道具以字符串的形式定义它的类型，
	//持有表示它有效时长的持续时间与表示当前是否被激活的属性。
	//在Breakout中，我们将添加4种增益道具与2种负面道具：
	// https://learnopengl-cn.github.io/img/06/Breakout/08/powerups.png
	/*
	Speed: 增加小球20%的速度
	Sticky: 当小球与玩家挡板接触时，小球会保持粘在挡板上的状态直到再次按下空格键，这可以让玩家在释放小球前找到更合适的位置
	Pass-Through: 非实心砖块的碰撞处理被禁用，使小球可以穿过并摧毁多个砖块
	Pad-Size-Increase: 增加玩家挡板50像素的宽度
	Confuse: 短时间内激活confuse后期特效，迷惑玩家
	Chaos: 短时间内激活chaos后期特效，使玩家迷失方向
	*/

	//与关卡中的砖块纹理类似，每个道具纹理都是完全灰度的，
	//这使得我们在将其与颜色向量相乘时可以保持色彩的平衡。

	//因为我们需要跟踪游戏中被激活的道具的类型、持续时间、相关效果等状态，
	//所以我们将它们存储在一个容器内：
	/*
	class Game
	{
	public:
		[...]
		std::vector<PowerUp>  PowerUps;
		[...]
		void SpawnPowerUps(GameObject &block);
		void UpdatePowerUps(GLfloat dt);
	};
	*/
	//我们还定义了两个管理道具的函数，
	//SpawnPowerUps在给定的砖块位置生成一个道具，
	//UpdatePowerUps管理所有当前被激活的道具。

	//SpawnPowerUps---------------------------------------------------------------

	//每次砖块被摧毁时我们希望以一定几率生成一个道具，这个功能可以在Game的SpawnPowerUps函数中找到：
	/*
	GLboolean ShouldSpawn(GLuint chance)
	{
		GLuint random = rand() % chance;
		return random == 0;
	}
	void Game::SpawnPowerUps(GameObject &block)
	{
		if (ShouldSpawn(75)) // 1/75的几率
			this->PowerUps.push_back(
				 PowerUp("speed", glm::vec3(0.5f, 0.5f, 1.0f), 0.0f, block.Position, tex_speed
			 ));
		if (ShouldSpawn(75))
			this->PowerUps.push_back(
				PowerUp("sticky", glm::vec3(1.0f, 0.5f, 1.0f), 20.0f, block.Position, tex_sticky
			);
		if (ShouldSpawn(75))
			this->PowerUps.push_back(
				PowerUp("pass-through", glm::vec3(0.5f, 1.0f, 0.5f), 10.0f, block.Position, tex_pass
			));
		if (ShouldSpawn(75))
			this->PowerUps.push_back(
				PowerUp("pad-size-increase", glm::vec3(1.0f, 0.6f, 0.4), 0.0f, block.Position, tex_size
			));
		if (ShouldSpawn(15)) // 负面道具被更频繁地生成
			this->PowerUps.push_back(
				PowerUp("confuse", glm::vec3(1.0f, 0.3f, 0.3f), 15.0f, block.Position, tex_confuse
			));
		if (ShouldSpawn(15))
			this->PowerUps.push_back(
				PowerUp("chaos", glm::vec3(0.9f, 0.25f, 0.25f), 15.0f, block.Position, tex_chaos
			));
	}
	*/
	//这样的SpawnPowerUps函数以一定几率（1/75普通道具，1/15负面道具）生成一个新的PowerUp对象，
	//并设置其属性。每种道具有特殊的颜色使它们更具有辨识度，
	//同时根据类型决定其持续时间的秒数，若值为0.0f则表示它持续无限长的时间。
	//除此之外，每个道具初始化时传入被摧毁砖块的位置与上一小节给出的对应纹理。
	int next;

	//激活道具--------------------------------------------------------------------

	//接下来我们更新游戏的DoCollisions函数使它不只检查小球与砖块和挡板的碰撞，
	//还检查挡板与所有未被销毁的道具的碰撞。
	//注意我们在砖块被摧毁的同时调用SpawnPowerUps函数。
	/*
	void Game::DoCollisions()
	{
		for (GameObject &box : this->Levels[this->Level].Bricks)
		{
			if (!box.Destroyed)
			{
				Collision collision = CheckCollision(*Ball, box);
				if (std::get<0>(collision))
				{
					if (!box.IsSolid)
					{
						box.Destroyed = GL_TRUE;
						this->SpawnPowerUps(box);
					}
					[...]
				}
			}
		}
		[...]
		for (PowerUp &powerUp : this->PowerUps)
		{
			if (!powerUp.Destroyed)
			{
				if (powerUp.Position.y >= this->Height)
					powerUp.Destroyed = GL_TRUE;
				if (CheckCollision(*Player, powerUp))
				{   // 道具与挡板接触，激活它！
					ActivatePowerUp(powerUp);
					powerUp.Destroyed = GL_TRUE;
					powerUp.Activated = GL_TRUE;
				}
			}
		}
	}
	*/
	//对所有未被销毁的道具，我们检查它是否接触到了屏幕底部或玩家挡板，
	//无论哪种情况我们都销毁它，但当道具与玩家挡板接触时，激活这个道具。

	//激活道具的操作可以通过将其Activated属性设为true来完成，
	//实现其效果则需要将它传给ActivatePowerUp函数：
	/*
	void ActivatePowerUp(PowerUp &powerUp)
	{
		// 根据道具类型发动道具
		if (powerUp.Type == "speed")
		{
			Ball->Velocity *= 1.2;
		}
		else if (powerUp.Type == "sticky")
		{
			Ball->Sticky = GL_TRUE;
			Player->Color = glm::vec3(1.0f, 0.5f, 1.0f);
		}
		else if (powerUp.Type == "pass-through")
		{
			Ball->PassThrough = GL_TRUE;
			Ball->Color = glm::vec3(1.0f, 0.5f, 0.5f);
		}
		else if (powerUp.Type == "pad-size-increase")
		{
			Player->Size.x += 50;
		}
		else if (powerUp.Type == "confuse")
		{
			if (!Effects->Chaos)
				Effects->Confuse = GL_TRUE; // 只在chaos未激活时生效，chaos同理
		}
		else if (powerUp.Type == "chaos")
		{
			if (!Effects->Confuse)
				Effects->Chaos = GL_TRUE;
		}
	}
	*/

	//ActivatePowerUp的目的正如其名称，
	//它按本章教程之前所预设的那样激活了一个道具的效果。
	//我们检查道具的类型并相应地改变游戏状态。
	//对于Sticky和Pass-through效果，我们也相应地改变了挡板和小球的颜色来给玩家一些当前被激活了哪种效果的反馈。

	//因为Sticky和Pass-through效果稍微改变了一些原有的游戏逻辑，
	//所以我们将这些效果作为属性存储在小球对象中，这样我们可以根据小球当前激活了什么效果而改变游戏逻辑。
	//我们只在BallObject的头文件中增加了两个属性，但为了完整性下面给出了更新后的代码：

	//这样我们可以通过改动DoCollisions函数中小球与挡板碰撞的代码便捷地实现Sticky效果。
	/*
	if (!Ball->Stuck && std::get<0>(result))
	{
		[...]
		Ball->Stuck = Ball->Sticky;
	}
	*/

	//在这里我们将小球的Stuck属性设置为它自己的Sticky属性，
	//若Stikcy效果被激活，那么小球则会在与挡板接触时粘在上面，
	//玩家不得不再次按下空格键才能释放它。
	int space;

	//在同样的DoCollisions函数中还有个为了实现Pass-through效果的类似小改动。
	//当小球的PassThrough属性被设置为true时，我们不对非实心砖块做碰撞处理操作。
	/*
	Direction dir = std::get<1>(collision);
	glm::vec2 diff_vector = std::get<2>(collision);
	if (!(Ball->PassThrough && !box.IsSolid))
	{
		if (dir == LEFT || dir == RIGHT) // 水平碰撞
		{
			[...]
		}
		else
		{
			[...]
		}
	}
	*/

	//其他效果可以通过简单的更改游戏的状态来实现，
	//如小球的速度、挡板的尺寸、PostProcessor对象的效果。

	//更新道具--------------------------------------------------------------------

	//现在剩下要做的就是保证道具生成后可以移动，并且在它们的持续时间用尽后失效，否则道具将永远保持激活状态。

	//在游戏的UpdatePowerUps函数中，我们根据道具的速度移动它，并减少已激活道具的持续时间，
	//每当时间减少至小于0时，我们令其失效，并恢复相关变量的状态。
	/*
	void Game::UpdatePowerUps(GLfloat dt)
	{
		for (PowerUp &powerUp : this->PowerUps)
		{
			powerUp.Position += powerUp.Velocity * dt;
			if (powerUp.Activated)
			{
				powerUp.Duration -= dt;

				if (powerUp.Duration <= 0.0f)
				{
					// 之后会将这个道具移除
					powerUp.Activated = GL_FALSE;
					// 停用效果
					if (powerUp.Type == "sticky")
					{
						if (!isOtherPowerUpActive(this->PowerUps, "sticky"))
						{   // 仅当没有其他sticky效果处于激活状态时重置，以下同理
							Ball->Sticky = GL_FALSE;
							Player->Color = glm::vec3(1.0f);
						}
					}
					else if (powerUp.Type == "pass-through")
					{
						if (!isOtherPowerUpActive(this->PowerUps, "pass-through"))
						{
							Ball->PassThrough = GL_FALSE;
							Ball->Color = glm::vec3(1.0f);
						}
					}
					else if (powerUp.Type == "confuse")
					{
						if (!isOtherPowerUpActive(this->PowerUps, "confuse"))
						{
							Effects->Confuse = GL_FALSE;
						}
					}
					else if (powerUp.Type == "chaos")
					{
						if (!isOtherPowerUpActive(this->PowerUps, "chaos"))
						{
							Effects->Chaos = GL_FALSE;
						}
					}
				}
			}
		}
		this->PowerUps.erase(std::remove_if(this->PowerUps.begin(), this->PowerUps.end(),
			[](const PowerUp &powerUp) { return powerUp.Destroyed && !powerUp.Activated; }
		), this->PowerUps.end());
	}
	*/

	//你可以看到对于每个效果，我们通过将相关元素重置来停用它。
	//我们还将PowerUp的Activated属性设为false，
	//在UpdatePowerUps结束时，我们通过循环PowerUps容器，
	//若一个道具被销毁切被停用，则移除它。
	//我们在算法开头使用remove_if函数，通过给定的lamda表达式消除这些对象。

	//remove_if函数将lamda表达式为true的元素移动至容器的末尾并返回一个迭代器指向应被移除的元素范围的开始部分。
	//容器的erase函数接着擦除这个迭代器指向的元素与容器末尾元素之间的所有元素。

	//可能会发生这样的情况：当一个道具在激活状态时，另一个道具与挡板发生了接触。
	//在这种情况下我们有超过1个在当前PowerUps容器中处于激活状态的道具。
	//然后，当这些道具中的一个被停用时，我们不应使其效果失效因为另一个相同类型的道具仍处于激活状态。出于这个原因，我们使用isOtherPowerUpActive检查是否有同类道具处于激活状态。
	//只有当它返回false时，我们才停用这个道具的效果。
	//这样，给定类型的道具的持续时间就可以延长至最近一次被激活后的持续时间。
	/*
	GLboolean IsOtherPowerUpActive(std::vector<PowerUp> &powerUps, std::string type)
	{
		for (const PowerUp &powerUp : powerUps)
		{
			if (powerUp.Activated)
				if (powerUp.Type == type)
					return GL_TRUE;
		}
		return GL_FALSE;
	}
	*/
	//这个函数简单地检查是否有同类道具处于激活状态，如果有则返回GL_TRUE。
	int space;
	//最后剩下的一件事便是渲染道具：
	/*
	void Game::Render()
	{
		if (this->State == GAME_ACTIVE)
		{
			[...]
			for (PowerUp &powerUp : this->PowerUps)
				if (!powerUp.Destroyed)
					powerUp.Draw(*Renderer);
			[...]
		}
	}    
	*/

	//结合所有的这些功能，我们有了一个可以运作的道具系统，
	//它不仅使游戏更有趣，还使游戏更具有挑战性。
	//它看上去会像这样：https://learnopengl.com/video/in-practice/breakout/powerups.mp4
}

//音效
namespace Audio
{
	//无论我们将游戏音量调到多大，我们都不会听到来自游戏的任何音效。
	//我们已经展示了这么多内容，但没有任何音频，游戏仍显得有些空洞。
	//在本节教程中，我们将解决这个问题。

	//OpenGL不提供关于音频的任何支持。
	//我们不得不手动将音频加载为字节格式，处理并将其转化为音频流，
	//并适当地管理多个音频流以供我们的游戏使用。
	//然而这有一些复杂，并且需要一些底层的音频工程知识。

	//如果你乐意，你可以手动加载来自多种扩展名的音频文件的音频流。
	//然而，我们将使用被称为irrKlang的音频管理库。

	//irrKlang-----------------------------------------------------------------------------------------

	//IrrKlang是一个可以播放WAV，MP3，OGG和FLAC文件的高级二维和三维（Windows，Mac OS X，Linux）声音引擎和音频库。
	//它还有一些可以自由调整的音频效果，如混响、延迟和失真。

	//3D音频意味着音频源可以有一个3D位置，然后根据相机到音频源的位置衰减音量，
	//使其在一个3D世界里显得自然（想想3D世界中的枪声，通常你可以从音效中听出它来自什么方向/位置）。

	//IrrKlang是一个易于使用的音频库，只需几行代码便可播放大多数音频文件，这使它成为我们Breakout游戏的完美选择。
	//请注意，irrKlang有一个有一定限制的证书：允许你将irrKlang用于非商业目的，
	//但是如果你想使用irrKlang商业版，就必须支付购买他们的专业版。
	//由于Breakout和本教程系列是非商业性的，所以我们可以自由地使用他们的标准库。

	//你可以从他们的下载页面下载irrKlang，我们将使用1.5版本。由于irrKlang是非开源的代码，因此我们不得不使用irrKlang为我们提供的任何东西。
	//幸运的是，他们有大量的预编译库文件，所以你们大多数人应该可以很好地使用它。

	//你需要引入了irrKlang的头文件，将他们的库文件（irrKlang.lib）添加到链接器设置中，
	//并将他们的dll文件复制到适当的目录下（通常和.exe在同一目录下）。
	//需要注意的是，如果你想要加载MP3文件，则还需要引入ikpMP3.dll文件。

	//添加音乐-----------------------------------------------------------------------------------------

	//为了这个游戏我特制了一个小小的音轨，让游戏更富有活力。
	//在这里你可以找到我们将要用作游戏背景音乐的音轨。 https://learnopengl.com/audio/in-practice/breakout/breakout.mp3
	//这个音轨会在游戏开始时播放并不断循环直到游戏结束。
	//你可以随意用自己的音频替换它，或者用喜欢的方式使用它。

	//利用irrKlang库将其添加到Breakout游戏里非常简单。我们引入相应的头文件，
	//创建irrKlang::ISoundEngine，用createIrrKlangDevice初始化它并使用这个引擎加载、播放音频：
	/*
	#include <irrklang/irrKlang.h>
	using namespace irrklang;

	ISoundEngine *SoundEngine = createIrrKlangDevice();

	void Game::Init()
	{
		[...]
		SoundEngine->play2D("audio/breakout.mp3", GL_TRUE);
	}
	*/

	//在这里，我们创建了一个SoundEngine，用于管理所有与音频相关的代码。
	//一旦我们初始化了引擎，便可以调用play2D函数播放音频。
	//第一个参数为文件名，第二个参数为是否循环播放。

	//这就是全部了！现在运行游戏会使你的耳机或扬声器迸发出声波。

	//添加音效-----------------------------------------------------------------------------------------

	//我们还没有结束，因为音乐本身并不能使游戏完全充满活力。
	//我们希望在游戏发生一些有趣事件时播放音效，作为给玩家的额外反馈，
	//如我们撞击砖块、获得道具时。
	//下面你可以找到我们需要的所有音效（来自freesound.org）：
	/*
	https://learnopengl.com/audio/in-practice/breakout/bleep.mp3
	https://learnopengl.com/audio/in-practice/breakout/solid.wav
	https://learnopengl.com/audio/in-practice/breakout/powerup.wav
	https://learnopengl.com/audio/in-practice/breakout/bleep.wav
	*/

	//无论在哪里发生碰撞，我们都会播放相应的音效。
	//我不会详细阐述每一行的代码，
	//你应该可以轻松地找到相应的添加音效的地方。

	//把这些集成在一起后我们的游戏显得更完整了，就像这样：https://learnopengl.com/video/in-practice/breakout/audio.mp4



}

//渲染文本
namespace RenderText
{
	//本教程中将通过增加生命值系统、获胜条件和渲染文本形式的反馈来对游戏做最后的完善。
	//本教程很大程度上是建立在之前的教程文本渲染基础之上，
	//因此如果没有看过的话，强烈建议您先一步一步学习之前的教程。

	//在Breakout中，所有的文本渲染代码都封装在一个名为TextRenderer的类中，
	//其中包含FreeType库的初始化、渲染配置和实际渲染代码等重要组成部分。
	//以下是TextRenderer类的代码：
	//textRender.h
	
	//以下是文本渲染的着色器
	//顶点着色器
	/*
	#version 330 core
	layout (location = 0) in vec4 vertex; // <vec2 pos, vec2 tex>
	out vec2 TexCoords;

	uniform mat4 projection;

	void main()
	{
		gl_Position = projection * vec4(vertex.xy, 0.0, 1.0);
		TexCoords = vertex.zw;
	} 
	*/
	//片段着色器
	/*
	#version 330 core
	in vec2 TexCoords;
	out vec4 color;

	uniform sampler2D text;
	uniform vec3 textColor;

	void main()
	{    
		vec4 sampled = vec4(1.0, 1.0, 1.0, texture(text, TexCoords).r);
		color = vec4(textColor, 1.0) * sampled;
	}  
	*/


}

//结语
/*
与仅仅是用OpenGL创建一个技术演示相比，这一整章的教程给我们了一次体验在此之上的更多内容的机会。
我们从零开始制作了一个2D游戏，并学习了如何对特定的底层图形学概念进行抽象、使用基础的碰撞检测技术、创建粒子、展示基于正射投影矩阵的场景。
所有的这些都使用了之前教程中讨论过的概念。
我们并没有真正地学习和使用OpenGL中新的、令人兴奋的图形技术，
更多的是在将所有知识整合至一个更大的整体中。

Breakout这样的一个简单游戏的制作可以被数千种方法完成，而我们的做法也只是其中之一。
随着游戏越来越庞大，你开始应用的抽象思想与设计模式就会越多。
如果希望进行更深入的学习与阅读，你可以在game programming patterns找到大部分的抽象思想与设计模式。
（译注：《游戏编程模式》一书国内已有中文翻译版，GPP翻译组译，人民邮电出版社）

请记住，编写出一个有着非常干净、考虑周全的代码的游戏是一件很困难的任务（几乎不可能）。
你只需要在编写游戏时使用在当时你认为正确的方法。
随着你对视频游戏开发的实践越来越多，你学习的新的、更好地解决问题的方法就越多。
不必因为编写“完美”代码的困难感到挫败，坚持编程吧！

优化------------------------------------------------------------------------------------------------------

这些教程的内容和目前已完成的游戏代码的关注点都在于如何尽可能简单地阐述概念，而没有深入地优化细节。
因此，很多性能相关的考虑都被忽略了。
为了在游戏的帧率开始下降时可以提高性能，我们将列出一些现代的2D OpenGL游戏中常见的改进方案。

*渲染精灵表单/纹理图谱(Sprite sheet / Texture atlas)：
	代替使用单个渲染精灵渲染单个纹理的渲染方式，我们将所有需要用到的纹理组合到单个大纹理中（如同位图字体），
	并用纹理坐标来选择合适的精灵与纹理。
	切换纹理状态是非常昂贵的操作，而使用这种方法让我们几乎可以不用在纹理间进行切换。
	除此之外，这样做还可以让GPU更有效率地缓存纹理，获得更快的查找速度。（译注：cache的局部性原理）
*实例化渲染：
	代替一次只渲染一个四边形的渲染方式，我们可以将想要渲染的所有四边形批量化，
	并使用实例化渲染在一次<>draw call中成批地渲染四边形。
	这很容易实现，因为每个精灵都由相同的顶点组成，不同之处只有一个模型矩阵(Model Matrix)，
	我们可以很容易地将其包含在一个实例化数组中。
	这样可以使OpenGL每帧渲染更多的精灵。
	实例化渲染也可以用来渲染粒子和字符字形。
*三角形带(Triangle Strips)：
	代替每次渲染两个三角形的渲染方式，我们可以用OpenGL的TRIANGLE_STRIP渲染图元渲染它们，
	只需4个顶点而非6个。这节约了三分之一需要传递给GPU的数据量。
*空间划分(Space partition)算法：
	当检查可能发生的碰撞时，我们将小球与当前关卡中的每一个砖块进行比较，
	这么做有些浪费CPU资源，因为我们可以很容易得知在这一帧中，大多数砖块都不会与小球很接近。
	使用BSP，八叉树(Octress)或k-d(imension)树等空间划分算法，
	我们可以将可见的空间划分成许多较小的区域，并判断小球是否在这个区域中，
	从而为我们省去大量的碰撞检查。
	对于Breakout这样的简单游戏来说，这可能是过度的，但对于有着更复杂的碰撞检测算法的复杂游戏，
	这些算法可以显著地提高性能。
*最小化状态间的转换：
	状态间的变化（如绑定纹理或切换着色器）在OpenGL中非常昂贵，因此你需要避免大量的状态变化。
	一种最小化状态间变化的方法是创建自己的状态管理器来存储OpenGL状态的当前值（比如绑定了哪个纹理），
	并且只在需要改变时进行切换，这可以避免不必要的状态变化。
	另外一种方式是基于状态切换对所有需要渲染的物体进行排序。
	首先渲染使用着色器A的所有对象，然后渲染使用着色器B的所有对象，以此类推。
	当然这可以扩展到着色器、纹理绑定、帧缓冲切换等。

这些应该可以给你一些关于，我们可以用什么样的的高级技巧进一步提高2D游戏性能地提示。
这也让你感受到了OpenGL的强大功能。
通过亲手完成大部分的渲染，我们对整个渲染过程有了完整的掌握，从而可以实现对过程的优化。
如果你对Breakout的性能并不满意，你可以把这些当做练习。

开始创作！------------------------------------------------------------------------------------------------

你已经看到了如何在OpenGL中创建一个简单的游戏，现在轮到你来创作属于自己的渲染/游戏程序了。
到目前为止我们讨论的许多技术都可以应用于大部分2D游戏中，
如渲染精灵、基础的碰撞检测、后期处理、文本渲染和粒子系统。
现在你可以将这些技术以你认为合理的方式进行组合与修改，并开发你自己的手制游戏吧！

*/