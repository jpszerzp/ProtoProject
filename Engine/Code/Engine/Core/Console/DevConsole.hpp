#pragma once
#include "Engine/Core/Thread/ThreadSafeQueue.hpp"
#include "Engine/Core/Rgba.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Input/InputSystem.hpp"

// Will be a singleston
class DevConsole 
{
public:
	DevConsole(); 
	~DevConsole(); 
	void ConfigureSections();
	void ConfigureMeshes();

	// Handles all input
	void Update(InputSystem* input, float deltaSeconds);
	void UpdateCursorBox(float deltaSeconds);
	void UpdateAutoComplete();
	void UpdateInputBuffer();
	bool CheckAutoCompleteMatch();
	void ConfigureDefaultAutoComplete();
	void ProcessInput(InputSystem* input);
	void RegisterConsoleHandler();

	// Renders the display
	void Render(Renderer* renderer); 
	void RenderFormatOutput(Renderer* renderer);
	//void RenderAutoComplete(Renderer* renderer);

	void Open(); 
	void Close();
	void InitializeCamera();
	bool IsOpen();	

public: 
	// excercise to the reader
	// add whatever members and methods you need to make it work
	static DevConsole* m_consoleInstance;
	bool m_isOpen;
	bool m_appQuit;
	bool m_isSelecting;
	bool m_inAutoCompleteMode;

	AABB2 m_cameraOrtho;
	AABB2 m_outputBox;
	AABB2 m_inputBox;
	AABB2 m_cursorBox;
	AABB2 m_autoCompleteBox;

	std::string m_inputBuffer;
	std::string m_lastCmdNameValid;
	std::vector<Rgba> m_outputColors;
	std::vector<std::string> m_outputText;
	std::vector<std::string> m_cmdHistory;
	std::vector<std::string> m_autoCompleteList;

	int m_cursorBlinkDir;
	int m_historyCounter;
	int m_autoCompleteCounter;
	int m_inputIndex;
	float m_uniformCellHeight;
	float m_cursorTimer;
	float m_selectionCursorRecordX;
	float m_rightArrowOffset;
	float m_leftArrowOffset;

	int m_cursorOffset = 0;

	BitmapFont* m_font;
	Camera* m_consoleCamera;

	const float ASPECT_SCALE = 0.5f;
	const float INPUTBOX_HEIGHT_RATIO = 0.03f;
	const float CURSOR_WIDTH_PERCENTAGE = 0.00125f;
	const int CMD_HISTORY_LOG_SIZE = 32;

	Mesh* m_backgroundMesh = nullptr;
	Mesh* m_inputBoxMesh = nullptr;
	Mesh* m_cursorMesh = nullptr;
	Mesh* m_inputBufferMesh = nullptr;
	std::vector<Mesh*> m_outputMeshes;
	ThreadSafeQueue<std::string> m_threadSafeMsg;

public:
	static DevConsole*  GetInstance(); 
	static void			DestroyConsole();								// managed by App.cpp	

	// getter
	std::string				  GetInputBuffer() const { return m_inputBuffer; }
	std::string				  GetLastCmdNameValid() const { return m_lastCmdNameValid; }
	std::vector<std::string>& GetCmdHistory() { return m_cmdHistory; }
	std::vector<std::string>  GetOutputTexts() const { return m_outputText; }
	bool					  GetAppShouldQuit() const { return m_appQuit; }

	// setter
	void SetFont(BitmapFont* font) { m_font = font; }
	void SetLastCmdNameValid(std::string name) {m_lastCmdNameValid = name;}
	void SetAppShouldQuit(bool value) {m_appQuit = value;}
	void SetCamOrtho(AABB2 ortho) { m_cameraOrtho = ortho; }
	void EnableAutoCompleteMode(bool value) {m_inAutoCompleteMode = value;}

	// buffer ops
	void RunInputBufferAsCommand();
	void RunSpecifiedCommand(std::string cmd);
	void AppendCharacterToInputBuffer(char character);
	void AppendInputBuffer(char character);
	void AppendFormatToOutputBuffer(std::string text);
	void AppendCmdHistoryLog(std::string cmd);
	void ClearOutputBufferFormat();
	int	 ClearSelectionArea();

	template<typename T>
	void DeleteVector(std::vector<T*>& vector);

	//void FillOutputBuffer(Rgba tint = Rgba::WHITE);
	void FillOutputBufferUnit(Rgba tint);
	void FillOutputBufferThreadSafe(Rgba tint);			// Deprecated
};


// Global functions

// I like this as a C function or a static 
// method so that it is easy for systems to check if the dev
// console is open.  
bool DevConsoleIsOpen(); 

// Same as previous, be defaults to a color visible easily on your console
//void ConsolePrintf( const char* format, ... );				
void ConsolePrintfUnit(Rgba color, const char* format, ...);			
void ConsolePrintfThreadSafe(Rgba color, const char* format, ...);		// Deprecated


void BufferWriteToFile(const char* fp);


template<typename T>
void DevConsole::DeleteVector(std::vector<T*>& vector)
{
	for each (T* element in vector)
	{
		delete element;
		element = nullptr;
	}

	vector.clear();
}