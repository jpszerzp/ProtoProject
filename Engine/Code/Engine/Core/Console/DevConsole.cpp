#include "Engine/Renderer/Window.hpp"
#include "Engine/Renderer/Mesh.hpp"
#include "Engine/Core/Console/Command.hpp"
#include "Engine/Core/Console/DevConsole.hpp"
#include "Engine/Core/Util/StringUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Net/RemoteCommandService.hpp"

#include <math.h>
#include <ctype.h>
#include <stdarg.h>
#include <fstream>

DevConsole* DevConsole::m_consoleInstance = nullptr;

static bool ConsoleHandleMsg(unsigned int msg, size_t wparam, size_t)
{
	unsigned char keyCode = (unsigned char)wparam;
	int ascii = toascii(keyCode);
	char character = char(ascii);

	if (DevConsoleIsOpen())
	{
		switch (msg)
		{
		case WM_CHAR:
			switch (keyCode)
			{
			case VK_ESCAPE:
				return true;		// do not modify buffer 
			case VK_RETURN:
				return true;
			case VK_UP:
				return true;
			case VK_DOWN:
				return true;
			case VK_RIGHT:
				return true;
			case VK_LEFT:
				return true;
			case VK_BACK:
				return true;
				/*
			case VK_DELETE:
				return true;		// conflict with .?
				*/
			case VK_SHIFT:
				return true;
			default:
				break;
			}

			DevConsole::GetInstance()->AppendInputBuffer(character);
			return true;
		default:
			return false;
		}
	}

	return false;
}


DevConsole::DevConsole()
{
	m_isOpen = false;
	m_appQuit = false;
	m_isSelecting = false;

	m_cursorTimer = 0.f;
	m_cursorBlinkDir = 1;

	m_inputIndex = 0;
	m_inputBuffer = "";

	m_rightArrowOffset = 0.f;
	m_leftArrowOffset = 0.f;

	m_inAutoCompleteMode = false;
	ConfigureDefaultAutoComplete();

	m_font = nullptr;
	m_consoleCamera = nullptr;
	m_historyCounter = -1;

	InitializeCamera();
}


void DevConsole::ConfigureSections()
{
	float consoleHeight = m_cameraOrtho.GetDimensions().y;

	m_inputBox = AABB2(m_cameraOrtho.mins, Vector2(m_cameraOrtho.maxs.x, m_cameraOrtho.mins.y + consoleHeight * INPUTBOX_HEIGHT_RATIO));
	m_outputBox = AABB2(Vector2(m_inputBox.mins.x, m_inputBox.maxs.y), m_cameraOrtho.maxs);
	m_uniformCellHeight = m_inputBox.GetDimensions().y;
}


void DevConsole::ConfigureMeshes()
{
	// background img
	Vector3 bl = m_cameraOrtho.mins.ToVector3(0.f);
	Vector3 br = bl + Vector3(m_cameraOrtho.GetDimensions().x, 0.f, 0.f);
	Vector3 tl = bl + Vector3(0.f, m_cameraOrtho.GetDimensions().y, 0.f);
	Vector3 tr = m_cameraOrtho.maxs.ToVector3(0.f);
	Rgba tint = Rgba::WHITE_HALF_OPACITY;
	m_backgroundMesh = Mesh::CreateQuadImmediate(VERT_PCU, bl, br, tl, tr, tint);

	// input box
	bl = m_inputBox.mins.ToVector3(0.f);
	br = bl + Vector3(m_inputBox.GetDimensions().x, 0.f, 0.f);
	tl = bl + Vector3(0.f, m_inputBox.GetDimensions().y, 0.f);
	tr = m_inputBox.maxs.ToVector3(0.f);
	tint = Rgba::BLACK;
	m_inputBoxMesh = Mesh::CreateQuadImmediate(VERT_PCU, bl, br, tl, tr, tint);

	// cursor box mesh is updated every frame

	// input buffer mesh updated every frame
}


DevConsole::~DevConsole()
{
	delete m_consoleCamera;
	m_consoleCamera = nullptr;

	delete m_backgroundMesh;
	m_backgroundMesh = nullptr;

	delete m_inputBoxMesh;
	m_inputBoxMesh = nullptr;

	delete m_cursorMesh;
	m_cursorMesh = nullptr;

	delete m_inputBufferMesh;
	m_inputBufferMesh = nullptr;

	DeleteVector(m_outputMeshes);
}


void DevConsole::RunSpecifiedCommand(std::string cmd)
{
	CommandRun(cmd.c_str());
	AppendCmdHistoryLog(cmd);
	m_inputBuffer.clear();
	m_cursorOffset = 0;
}


void DevConsole::RunInputBufferAsCommand()
{
	if (!m_inputBuffer.empty())
	{
		CommandRun(m_inputBuffer.c_str());
		AppendCmdHistoryLog(m_inputBuffer);
		m_inputBuffer.clear();
		m_cursorOffset = 0;
	}
}


void DevConsole::RegisterConsoleHandler()
{
	Window* theWindow = Window::GetInstance();
	theWindow->RegisterHandler(ConsoleHandleMsg);
}


void DevConsole::InitializeCamera()
{
	if (!m_consoleCamera)
	{
		Renderer* theRenderer = Renderer::GetInstance();
		m_consoleCamera = new Camera();
		m_consoleCamera->SetColorTarget(theRenderer->GetDefaultColorTarget());
		m_consoleCamera->SetDepthStencilTarget(theRenderer->GetDefaultDepthTarget());
	}
}


void DevConsole::ConfigureDefaultAutoComplete()
{
	m_autoCompleteBox = AABB2();
	m_autoCompleteList.clear();
	m_autoCompleteCounter = -1;
}


int DevConsole::ClearSelectionArea()
{
	int selectionStartIndex = -1;
	int selectionEndIndex = -1;
	bool searchFinished = false;
	int eraseDirection = 0;

	if (m_selectionCursorRecordX < m_cursorBox.mins.x)
	{
		eraseDirection = -1;

		if (m_selectionCursorRecordX == 0)
		{
			selectionStartIndex = 0;
		}

		for (int charCount = 0; charCount < m_inputBuffer.length(); ++charCount)
		{
			float charWidth = m_uniformCellHeight * ASPECT_SCALE;
			float minX = charCount * charWidth;
			if ((minX + charWidth) == m_selectionCursorRecordX)
			{
				selectionStartIndex = charCount + 1;
			}

			if ((minX + charWidth) == m_cursorBox.mins.x)
			{
				selectionEndIndex = charCount;
				searchFinished = true;
			}

			if (searchFinished)
				break;
		}
	}
	else
	{
		eraseDirection = 1;

		if (m_cursorBox.mins.x == 0)
		{
			selectionStartIndex = 0;
		}

		for (int charCount = 0; charCount < m_inputBuffer.length(); ++charCount)
		{
			float charWidth = m_uniformCellHeight * ASPECT_SCALE;
			float minX = charCount * charWidth;
			if ((minX + charWidth) == m_cursorBox.mins.x)
			{
				selectionStartIndex = charCount + 1;
			}

			if ((minX + charWidth) == m_selectionCursorRecordX)
			{
				selectionEndIndex = charCount;
				searchFinished = true;
			}

			if (searchFinished)
				break;
		}
	}

	int erasedLength = selectionEndIndex - selectionStartIndex + 1;
	m_inputBuffer.erase(selectionStartIndex, erasedLength);
	m_inAutoCompleteMode = CheckAutoCompleteMatch();				// check for auto complete status when input buffer is changed
	m_isSelecting = false;
	if (eraseDirection == 1)
	{
		m_rightArrowOffset += erasedLength;
	}

	return erasedLength;
}


void DevConsole::ProcessInput(InputSystem* input)
{
	if (input->WasKeyJustPressed(InputSystem::KEYBOARD_BACKSPACE))
	{
		if (!m_inputBuffer.empty())
		{
			int size = (int)m_inputBuffer.size();
			std::string former = m_inputBuffer.substr(0, m_cursorOffset - 1);
			std::string latter = m_inputBuffer.substr(m_cursorOffset, size - m_cursorOffset + 1);
			m_inputBuffer = former + latter;
			m_cursorOffset--;
		}
	}
	else if (input->WasKeyJustPressed(InputSystem::KEYBOARD_ESC))
	{
		if (!m_inputBuffer.empty())
		{
			m_inputBuffer.clear();
			m_cursorOffset = 0;
		}
		else 
		{
			m_isOpen = false;

			input->MouseLockCursor(true);
		}
	}
	else if (input->WasKeyJustPressed(InputSystem::KEYBOARD_ENTER))
	{
		// run input as buffer
		// push the cmd to history
		// clear input buffer and reset cursor
		RunInputBufferAsCommand();

		// update history counter
		m_historyCounter = (int)m_cmdHistory.size();
	}
	else if (input->WasKeyJustPressed(InputSystem::KEYBOARD_LEFT_ARROW))
	{
		if (m_cursorOffset != 0)
		{
			m_cursorOffset--;
		}
	}
	else if (input->WasKeyJustPressed(InputSystem::KEYBOARD_RIGHT_ARROW))
	{
		if (m_cursorOffset != m_inputBuffer.size())
		{
			m_cursorOffset++;
		}
	}
	else if (input->WasKeyJustPressed(InputSystem::KEYBOARD_UP_ARROW))
	{
		int wantedCounter = m_historyCounter - 1;
		if (wantedCounter >= 0)
		{
			m_historyCounter = wantedCounter;
		}
		std::string historyCmd = m_cmdHistory[m_historyCounter];
		m_inputBuffer = historyCmd;
		m_cursorOffset = (int)m_inputBuffer.length();
	}
	else if (input->WasKeyJustPressed(InputSystem::KEYBOARD_DOWN_ARROW))
	{
		int wantedCounter = m_historyCounter + 1;
		if (wantedCounter <= m_cmdHistory.size())
		{
			m_historyCounter = wantedCounter;
		}
		
		// if coming to end of history, clear input buffer
		// else, present history as normal 
		if (m_historyCounter == m_cmdHistory.size())
		{
			m_inputBuffer = "";
			m_cursorOffset = 0;
		}
		else
		{
			std::string historyCmd = m_cmdHistory[m_historyCounter];
			m_inputBuffer = historyCmd;
			m_cursorOffset = (int)m_inputBuffer.length();
		}
	}
}


void DevConsole::AppendInputBuffer(char character)
{
	//m_inputBuffer += character;
	int size = (int)m_inputBuffer.size();
	std::string former = m_inputBuffer.substr(0, m_cursorOffset) + character;
	std::string latter = m_inputBuffer.substr(m_cursorOffset, size - m_cursorOffset + 1);
	m_inputBuffer = former + latter;
	m_cursorOffset++;
}


void DevConsole::AppendCharacterToInputBuffer(char character)
{
	int erasedChars = 0;
	if (m_isSelecting)
	{
		erasedChars = ClearSelectionArea();
		//m_isSelecting = false;
	}

	// record, on relative position between cursor and selection start, has not been removed
	if (m_cursorBox.mins.x > m_selectionCursorRecordX)
	{
		//m_leftArrowOffset += (erasedChars - 1);

		// update cursor for insertion
		//UpdateCursorBox();
	}

	// find insertion index
	int insertedAfterIndex = -1;
	for (int charCount = 0; charCount < m_inputBuffer.length(); ++charCount)
	{
		float charWidth = m_uniformCellHeight * ASPECT_SCALE;
		float minX = charCount * charWidth;
		if ((minX + charWidth) == m_cursorBox.mins.x)
		{
			insertedAfterIndex = charCount;
			break;
		}
	}

	// insert
	if (insertedAfterIndex == -1)
	{
		m_inputBuffer = character + m_inputBuffer;
	}
	else
	{
		std::string first_half_str = m_inputBuffer.substr(0, insertedAfterIndex + 1);
		std::string second_half_str = "";
		if (insertedAfterIndex < (m_inputBuffer.size() - 1))
		{
			second_half_str = m_inputBuffer.substr(insertedAfterIndex + 1);
		}
		m_inputBuffer = first_half_str + character + second_half_str;
	}

	m_inAutoCompleteMode = CheckAutoCompleteMatch();
}


bool DevConsole::CheckAutoCompleteMatch()
{
	m_autoCompleteList = CommandAutoComplete(m_inputBuffer.c_str());
	return (!m_autoCompleteList.empty());
}


void DevConsole::UpdateAutoComplete()
{
	int numOfMatchCmd = (int)m_autoCompleteList.size();
	float autoCompelteBoxHeight = m_uniformCellHeight * numOfMatchCmd;
	m_autoCompleteBox = AABB2(Vector2(0.f, m_uniformCellHeight), Vector2(Window::GetInstance()->GetWindowWidth(), m_uniformCellHeight + autoCompelteBoxHeight));
}


void DevConsole::UpdateCursorBox(float deltaSeconds)
{
	if (m_cursorMesh != nullptr)
	{
		delete m_cursorMesh;
		m_cursorMesh = nullptr;
	}

	// create new cursor mesh based on updated cursor box
	m_cursorTimer += (m_cursorBlinkDir * deltaSeconds);

	if (abs(m_cursorTimer) > 0.2f)
	{
		m_cursorTimer = 0.f;
		m_cursorBlinkDir *= -1;
	}

	int inputSize = (int)m_inputBuffer.size();
	float inputWidth = m_font->GetStringWidth(m_inputBuffer, m_uniformCellHeight, ASPECT_SCALE);
	float charWidth = m_uniformCellHeight * ASPECT_SCALE;
	float cursorBLX = 0.f + inputWidth - (inputSize - m_cursorOffset) * charWidth;
	Vector2 cursorBL = Vector2(cursorBLX, 0.f);
	m_cursorBox = AABB2(cursorBL, cursorBL + Vector2(2.f, m_uniformCellHeight));

	Vector3 bl = m_cursorBox.mins.ToVector3(0.f);
	Vector3 br = bl + Vector3(m_cursorBox.GetDimensions().x, 0.f, 0.f);
	Vector3 tl = bl + Vector3(0.f, m_cursorBox.GetDimensions().y, 0.f);
	Vector3 tr = m_cursorBox.maxs.ToVector3(0.f);
	Rgba tint = Rgba::GREEN;
	m_cursorMesh = Mesh::CreateQuadImmediate(VERT_PCU, bl, br, tl, tr, tint);
}


void DevConsole::Update(InputSystem* input, float deltaSeconds)
{
	// Add msg to threaded msg queue at start of update??

	if (m_isOpen)
	{
		UpdateInputBuffer();
		ProcessInput(input);
		UpdateCursorBox(deltaSeconds);
	}
}


void DevConsole::UpdateInputBuffer()
{
	Renderer* renderer = Renderer::GetInstance();

	if (m_inputBufferMesh != nullptr)
	{
		delete m_inputBufferMesh;
		m_inputBufferMesh = nullptr;
	}

	if (m_inputBuffer != "")
	{
		Rgba textTint = Rgba::WHITE;
		BitmapFont* font = renderer->CreateOrGetBitmapFont("Data/Fonts/SquirrelFixedFont.png");
		m_inputBufferMesh = Mesh::CreateTextImmediate(textTint, m_inputBox.mins,
			font, m_uniformCellHeight, ASPECT_SCALE, m_inputBuffer, VERT_PCU);
	}
}

/*
void DevConsole::FillOutputBuffer(Rgba tint)
{
	Renderer* renderer = Renderer::GetInstance();
	int outputTextSize = (int)m_outputText.size();

	for (int outputIdx = 0; outputIdx < outputTextSize; ++outputIdx)
	{
		//Rgba textTint = Rgba::WHITE;
		BitmapFont* font = renderer->CreateOrGetBitmapFont("Data/Fonts/SquirrelFixedFont.png");
		Mesh* outputMesh = Mesh::CreateTextImmediate(tint, m_inputBox.mins +
			Vector2(0.f, (outputTextSize - outputIdx) * m_uniformCellHeight), font,
			m_uniformCellHeight, ASPECT_SCALE, m_outputText[outputIdx], VERT_PCU);
		m_outputMeshes.push_back(outputMesh);
	}
}
*/

void DevConsole::FillOutputBufferUnit(Rgba tint)
{
	if (!m_outputMeshes.empty())
	{
		// create a copy of existing text meshes
		std::vector<Mesh*> newMeshes;

		Renderer* renderer = Renderer::GetInstance();
		//int outputTextSize = (int)m_outputText.size();
		BitmapFont* font = renderer->CreateOrGetBitmapFont("Data/Fonts/SquirrelFixedFont.png");

		for (uint i = 0; i < m_outputMeshes.size(); ++i)
		{
			Mesh* oldMesh = m_outputMeshes[i];

			Rgba oldColor = oldMesh->m_textMeshColor;
			Vector2 oldDrawmin = oldMesh->m_textMeshDrawmin2;

			Mesh* newMesh = Mesh::CreateTextImmediate(oldColor, oldDrawmin + Vector2(0.f, m_uniformCellHeight), font,
				m_uniformCellHeight, ASPECT_SCALE, m_outputText[i], VERT_PCU);

			newMeshes.push_back(newMesh);
		}

		// delete all existing text meshes, if there is any
		if (!m_outputMeshes.empty())
		{
			for each (Mesh* textMesh in m_outputMeshes)
			{
				delete textMesh;
				textMesh = nullptr;
			}

			m_outputMeshes.clear();
		}

		m_outputMeshes = newMeshes;

		Mesh* newestMesh = Mesh::CreateTextImmediate(tint, m_inputBox.mins + Vector2(0.f, m_uniformCellHeight), font,
			m_uniformCellHeight, ASPECT_SCALE, m_outputText[m_outputText.size() - 1], VERT_PCU);
		m_outputMeshes.push_back(newestMesh);

		newMeshes.clear();
	}
	// first time ever pop mesh into data structure
	else
	{
		Renderer* renderer = Renderer::GetInstance();
		BitmapFont* font = renderer->CreateOrGetBitmapFont("Data/Fonts/SquirrelFixedFont.png");
		Mesh* exclusiveMesh = Mesh::CreateTextImmediate(tint, m_inputBox.mins + Vector2(0.f, m_uniformCellHeight), font,
			m_uniformCellHeight, ASPECT_SCALE, m_outputText[0], VERT_PCU);
		m_outputMeshes.push_back(exclusiveMesh);
	}
}

void DevConsole::FillOutputBufferThreadSafe(Rgba tint)
{
	Renderer* renderer = Renderer::GetInstance();
	int outputTextSize = (int)m_threadSafeMsg.m_data.size();

	for (int outputIdx = 0; outputIdx < outputTextSize; ++outputIdx)
	{
		//Rgba textTint = Rgba::WHITE;
		BitmapFont* font = renderer->CreateOrGetBitmapFont("Data/Fonts/SquirrelFixedFont.png");
		Mesh* outputMesh = Mesh::CreateTextImmediate(tint, m_inputBox.mins +
			Vector2(0.f, (outputTextSize - outputIdx) * m_uniformCellHeight), font,
			m_uniformCellHeight, ASPECT_SCALE, m_threadSafeMsg.m_data[outputIdx], VERT_PCU);
		m_outputMeshes.push_back(outputMesh);
	}
}

void DevConsole::Render(Renderer* renderer)
{
	if (m_isOpen)
	{
		renderer->SetCamera(m_consoleCamera);
		renderer->GetCurrentCamera()->SetProjectionOrtho(m_cameraOrtho.mins, m_cameraOrtho.maxs);

		Shader* shader = nullptr;
		Texture* texture = nullptr;

		// Render output buffer - content from threaded or non-threaded consoleprintf
		shader = renderer->CreateOrGetShader("cutout_nonmodel");
		texture = renderer->CreateOrGetTexture("Data/Fonts/SquirrelFixedFont.png");
		renderer->UseShader(shader);
		renderer->SetTexture2D(0, texture);
		renderer->SetSampler2D(0, texture->GetSampler());
		for (int outputMeshIdx = 0; outputMeshIdx < m_outputMeshes.size(); ++outputMeshIdx)
		{
			Mesh* outputMesh = m_outputMeshes[outputMeshIdx];
			renderer->DrawMesh(outputMesh);
		}

		// render rcs status
		RCS* rcs = RCS::GetInstance();
		shader = renderer->CreateOrGetShader("cutout_nonmodel");
		texture = renderer->CreateOrGetTexture("Data/Fonts/SquirrelFixedFont.png");
		renderer->UseShader(shader);
		renderer->SetTexture2D(0, texture);
		renderer->SetSampler2D(0, texture->GetSampler());
		rcs->Render(renderer);

		// Render cursor
		if (m_cursorTimer >= 0.f)
		{
			shader = renderer->CreateOrGetShader("2d_direct_opague");
			texture = renderer->CreateOrGetTexture("Data/Images/white.png");
			renderer->UseShader(shader);
			renderer->SetTexture2D(0,  texture);
			renderer->SetSampler2D(0, texture->GetSampler());
			renderer->DrawMesh(m_cursorMesh);
		}

		// Render input buffer
		if (m_inputBufferMesh != nullptr)
		{
			shader = renderer->CreateOrGetShader("2d_direct_opague");
			texture = renderer->CreateOrGetTexture("Data/Fonts/SquirrelFixedFont.png");
			renderer->UseShader(shader);
			renderer->SetTexture2D(0, texture);
			renderer->SetSampler2D(0, texture->GetSampler());
			renderer->DrawMesh(m_inputBufferMesh);
		}

		// Render input box 
		shader = renderer->CreateOrGetShader("2d_direct_opague");
		texture = renderer->CreateOrGetTexture("Data/Images/white.png");
		renderer->UseShader(shader);
		renderer->SetTexture2D(0,  texture);
		renderer->SetSampler2D(0, texture->GetSampler());
		renderer->DrawMesh(m_inputBoxMesh);

		// Render background img
		shader = renderer->CreateOrGetShader("2d_direct_alpha");
		texture = renderer->CreateOrGetTexture("Data/Images/Artorias.png");
		renderer->UseShader(shader);
		renderer->SetTexture2D(0, texture);
		renderer->SetSampler2D(0, texture->GetSampler());
		renderer->DrawMesh(m_backgroundMesh);
	}
}


//void DevConsole::RenderAutoComplete(Renderer* renderer)
//{
//	//renderer->UseShaderProgram(renderer->CreateOrGetShaderProgram("projection_color"));
//	renderer->UseShader(renderer->CreateOrGetShader("mp_color"));
//	renderer->DrawAABB(m_autoCompleteBox, Rgba::BLACK);
//
//	//renderer->UseShaderProgram(renderer->CreateOrGetShaderProgram("projection_textured"));
//	renderer->UseShader(renderer->CreateOrGetShader("mp_tex"));
//	for (std::vector<std::string>::size_type matchedCmdCount = 0; matchedCmdCount < m_autoCompleteList.size(); ++matchedCmdCount)
//	{
//		std::string cmdName = m_autoCompleteList[matchedCmdCount];
//		std::string cmdDoc = FindCmdDocWithName(cmdName);
//
//		AABB2 nameBox = AABB2(Vector2(0.f, m_uniformCellHeight * (matchedCmdCount + 1)), Vector2(m_font->GetStringWidth(cmdName, m_uniformCellHeight, ASPECT_SCALE), m_uniformCellHeight * (matchedCmdCount + 2)));
//		float docStrWidth = m_font->GetStringWidth(cmdDoc, m_uniformCellHeight, ASPECT_SCALE);
//		Vector2 docBoxMax = Vector2(Window::GetInstance()->GetWindowWidth(), m_uniformCellHeight * (matchedCmdCount + 2));
//		Vector2 docBoxMin = Vector2(Window::GetInstance()->GetWindowWidth() - docStrWidth, m_uniformCellHeight * (matchedCmdCount + 1));
//		
//		if (m_autoCompleteCounter == matchedCmdCount)
//		{
//			renderer->DrawTextInBox2D(cmdName, 0.f, 0.f, m_uniformCellHeight, nameBox, TEXT_DRAW_OVERRUN, Rgba::YELLOW, ASPECT_SCALE, m_font);
//			renderer->DrawTextInBox2D(cmdDoc, 0.f, 0.f, m_uniformCellHeight, AABB2(docBoxMin, docBoxMax), TEXT_DRAW_OVERRUN, Rgba::WHITE, ASPECT_SCALE, m_font);
//
//			// auto alignment/complete of characters
//			int incompleteNameLength = static_cast<int>(m_inputBuffer.length());
//			int cmdNameLength = static_cast<int>(m_autoCompleteList[m_autoCompleteCounter].length());
//			std::string completedName = m_autoCompleteList[m_autoCompleteCounter].substr(incompleteNameLength, cmdNameLength - incompleteNameLength);
//			Vector2 hintBoxMin = Vector2(m_font->GetStringWidth(m_inputBuffer, m_uniformCellHeight, ASPECT_SCALE), 0.f);
//			Vector2 hintBoxMax = Vector2(m_font->GetStringWidth(completedName, m_uniformCellHeight, ASPECT_SCALE) + hintBoxMin.x, m_uniformCellHeight);
//			AABB2 hintBox = AABB2(hintBoxMin, hintBoxMax);
//
//			//renderer->UseShaderProgram(renderer->CreateOrGetShaderProgram("projection_color"));
//			renderer->UseShader(renderer->CreateOrGetShader("mp_color"));
//			renderer->DrawAABB(hintBox, Rgba::WHITE_HALF_OPACITY);
//			//renderer->UseShaderProgram(renderer->CreateOrGetShaderProgram("projection_textured"));
//			renderer->UseShader(renderer->CreateOrGetShader("mp_tex"));
//			renderer->DrawTextInBox2D(completedName, 0.f, 0.f, m_uniformCellHeight, hintBox, TEXT_DRAW_OVERRUN, Rgba::WHITE, ASPECT_SCALE, m_font);
//		}
//		else
//		{
//			renderer->DrawTextInBox2D(cmdName, 0.f, 0.f, m_uniformCellHeight, nameBox, TEXT_DRAW_OVERRUN, Rgba::WHITE_HALF_OPACITY, ASPECT_SCALE, m_font);
//			renderer->DrawTextInBox2D(cmdDoc, 0.f, 0.f, m_uniformCellHeight, AABB2(docBoxMin, docBoxMax), TEXT_DRAW_OVERRUN, Rgba::WHITE_HALF_OPACITY, ASPECT_SCALE, m_font);
//		}
//	}
//}


void DevConsole::RenderFormatOutput(Renderer*)
{
	Vector2 highestLineDrawMin = m_outputBox.mins + Vector2(0.f, (m_outputText.size() - 1) * m_uniformCellHeight);				// height of input box is height of one line

	for(std::vector<std::string>::size_type textLineCount = 0; textLineCount < m_outputText.size(); ++textLineCount)
	{
		std::string content = m_outputText[textLineCount];
		//Rgba color = m_outputColors[textLineCount];

		//Vector2 lineDrawMin = Vector2(HighestLineDrawMin.x, HighestLineDrawMin.y - textLineCount * m_uniformCellHeight);

		//if (m_lastCmdNameValid.find("help") != std::string::npos)
		//{
		//	if (content != "help error: INVALID COMMAND")
		//	{
		//		size_t indexFirstSpace = nth_match_str(content, " ", 1);
		//		std::string cmdName = content.substr(0, indexFirstSpace);
		//		size_t indexSecondSpace = nth_match_str(content, " ", 2);
		//		std::string argList = content.substr(indexFirstSpace, indexSecondSpace - 1 - indexFirstSpace);
		//		std::string cmdDoc = content.substr(indexSecondSpace - 1);

		//		Vector2 argListDrawmin = lineDrawMin + Vector2(m_font->GetStringWidth(cmdName, m_uniformCellHeight, ASPECT_SCALE), 0.f);
		//		renderer->DrawTextInBox2D(cmdName, 0.f, 0.f, m_uniformCellHeight, AABB2(lineDrawMin, argListDrawmin + Vector2(0.f, m_uniformCellHeight)), TEXT_DRAW_OVERRUN,
		//			color, ASPECT_SCALE, m_font);
		//		Vector2 cmdDocDrawmin = argListDrawmin + Vector2(m_font->GetStringWidth(argList, m_uniformCellHeight, ASPECT_SCALE), 0.f);
		//		renderer->DrawTextInBox2D(argList, 0.f, 0.f, m_uniformCellHeight, AABB2(argListDrawmin, cmdDocDrawmin + Vector2(0.f, m_uniformCellHeight)), TEXT_DRAW_OVERRUN,
		//			Rgba::CYAN, ASPECT_SCALE, m_font);
		//		renderer->DrawTextInBox2D(cmdDoc, 0.f, 0.f, m_uniformCellHeight, AABB2(cmdDocDrawmin, cmdDocDrawmin + Vector2(m_font->GetStringWidth(cmdDoc, m_uniformCellHeight, ASPECT_SCALE), m_uniformCellHeight)), 
		//			TEXT_DRAW_OVERRUN, Rgba::MEGENTA, ASPECT_SCALE, m_font);
		//	}
		//	else
		//	{
		//		renderer->DrawTextInBox2D(content, 0.f, 0.f, m_uniformCellHeight, AABB2(lineDrawMin, lineDrawMin + Vector2(m_font->GetStringWidth(content, m_uniformCellHeight, ASPECT_SCALE), m_uniformCellHeight)), 
		//			TEXT_DRAW_OVERRUN, color, ASPECT_SCALE, m_font);
		//	}
		//}
		//else
		//{
		//	renderer->DrawTextInBox2D(content, 0.f, 0.f, m_uniformCellHeight, AABB2(lineDrawMin, lineDrawMin + Vector2(m_font->GetStringWidth(content, m_uniformCellHeight, ASPECT_SCALE), m_uniformCellHeight)), 
		//		TEXT_DRAW_OVERRUN, color, ASPECT_SCALE, m_font);
		//}

		//Vector2 lineDrawMin = Vector2(highestLineDrawMin.x,
		//	highestLineDrawMin.y - textLineCount * m_uniformCellHeight);
		//renderer->DrawText2D(lineDrawMin, content, m_uniformCellHeight,
		//	Rgba::WHITE, ASPECT_SCALE, m_font);
	}
}


void DevConsole::DestroyConsole()
{
	delete m_consoleInstance;
	m_consoleInstance = nullptr;
}


void DevConsole::Open()
{
	m_isOpen = true;
}


void DevConsole::Close()
{
	m_isOpen = false;
}


bool DevConsole::IsOpen()
{
	return m_isOpen;
}


DevConsole* DevConsole::GetInstance()
{
	if (!m_consoleInstance)
	{
		m_consoleInstance = new DevConsole();
	}
	
	return m_consoleInstance;
}


void DevConsole::AppendFormatToOutputBuffer(std::string text)
{
	m_outputText.push_back(text);
	//m_outputColors.push_back(color);
}


void DevConsole::ClearOutputBufferFormat()
{
	m_outputText.clear();
	m_outputMeshes.clear();

	/*
	std::string str;
	while (m_threadSafeMsg.Dequeue(&str))
	{

	}
	*/
}


void DevConsole::AppendCmdHistoryLog(std::string cmd)
{
	if (std::find(m_cmdHistory.begin(), m_cmdHistory.end(), cmd) != m_cmdHistory.end())
	{
		std::vector<std::string>::size_type foundIndex = std::find(m_cmdHistory.begin(), m_cmdHistory.end(), cmd) - m_cmdHistory.begin();
		if (foundIndex != (m_cmdHistory.size() - 1))
		{
			std::string foundStrTemp = m_cmdHistory[foundIndex];
			for (std::vector<std::string>::size_type remainedCmdIndex = foundIndex + 1; remainedCmdIndex < m_cmdHistory.size(); ++remainedCmdIndex)
			{
				m_cmdHistory[remainedCmdIndex - 1] = m_cmdHistory[remainedCmdIndex];
			}
			m_cmdHistory[m_cmdHistory.size() - 1] = foundStrTemp;
		}
	}
	else
	{
		int historyLogSize = (int)m_cmdHistory.size();
		if ((historyLogSize + 1) > CMD_HISTORY_LOG_SIZE)
		{
			m_cmdHistory.erase(m_cmdHistory.begin());		// erase the front (oldest) cmd
			m_cmdHistory.push_back(cmd);
		}
		else
		{
			m_cmdHistory.push_back(cmd);
		}
	}
}


bool DevConsoleIsOpen()
{
	DevConsole* console = DevConsole::GetInstance();
	return console->IsOpen();
}

/*
void ConsolePrintf(char const *format, ... )
{
	char textLiteral[ 2048 ];
	va_list variableArgumentList;
	va_start( variableArgumentList, format );
	vsnprintf_s( textLiteral, 2048, _TRUNCATE, format, variableArgumentList );	
	va_end( variableArgumentList );
	textLiteral[ 2048 - 1 ] = '\0';					

	std::string formatted( textLiteral );

	DevConsole* console = DevConsole::GetInstance();
	console->AppendFormatToOutputBuffer(formatted);
}
*/

void ConsolePrintfUnit(Rgba color, const char* format, ...)
{
	DevConsole* console = DevConsole::GetInstance();

	char textLiteral[ 2048 ];
	va_list variableArgumentList;
	va_start( variableArgumentList, format );
	vsnprintf_s( textLiteral, 2048, _TRUNCATE, format, variableArgumentList );	
	va_end( variableArgumentList );
	textLiteral[ 2048 - 1 ] = '\0';					

	std::string formatted( textLiteral );

	console->AppendFormatToOutputBuffer(formatted);

	console->FillOutputBufferUnit(color);

	if (g_rcs->m_echo == 1)
	{
		if (g_rcs->m_rcsState == RCS_CLIENT)
		{
			g_rcs->SendMsg(0, 2, formatted.c_str());	// host is at idx 0
		}
		else if (g_rcs->m_rcsState == RCS_HOST)
		{

		}
	}
}


void ConsolePrintfThreadSafe(Rgba color, const char* format, ...)
{
	// copy formatted content into string
	char textLiteral[ 2048 ];
	va_list variableArgumentList;
	va_start( variableArgumentList, format );
	vsnprintf_s( textLiteral, 2048, _TRUNCATE, format, variableArgumentList );	
	va_end( variableArgumentList );
	textLiteral[ 2048 - 1 ] = '\0';					

	std::string formatted( textLiteral );

	// add into thread safe queue
	DevConsole* console = DevConsole::GetInstance();
	console->m_threadSafeMsg.Enqueue(formatted);
	//console->ClearOutputBufferFormat();
}


void BufferWriteToFile(const char* fp)
{
	std::ofstream myfile;
	myfile.open (fp);

	std::string res;
	std::vector<std::string> outputTexts = DevConsole::GetInstance()->GetOutputTexts();
	for (std::vector<std::string>::size_type outputTextCount = 0; outputTextCount < outputTexts.size(); ++outputTextCount)
	{
		res += outputTexts[outputTextCount];
		res += "\n";
	}

	myfile << res;
	myfile.close();
}