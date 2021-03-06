/*
 Copyright (C) 2012 by Ivan Safrin
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
*/

#include "PolycodeFrame.h"

PolycodeFrame::PolycodeFrame() : ScreenEntity() {

	modalChild = NULL;
	
	welcomeEntity = new ScreenEntity();
	addChild(welcomeEntity);
	welcomeImage = new ScreenImage("welcome.png");
	welcomeEntity->addChild(welcomeImage);
	welcomeEntity->snapToPixels = true;
	
	newProjectButton = new UIButton("Create A New Project!", 220);	
	newProjectButton->setPosition(230,80);
	newProjectButton->addEventListener(this, UIEvent::CLICK_EVENT);
	
	examplesButton = new UIButton("Browse Example Projects!", 220);	
	examplesButton->setPosition(460,80);
	examplesButton->addEventListener(this, UIEvent::CLICK_EVENT);
	
	welcomeEntity->addChild(newProjectButton);
	welcomeEntity->addChild(examplesButton);
	
	editorHolder = new ScreenEntity();
	addChild(editorHolder);
		
	
	projectBrowser = new PolycodeProjectBrowser();
	addChild(projectBrowser);
	
	projectBrowser->treeContainer->getRootNode()->addEventListener(this, UITreeEvent::DRAG_START_EVENT);
		
	topBarBg = new ScreenShape(ScreenShape::SHAPE_RECT, 2,2);
	topBarBg->setColor(0,0,0,1);
	topBarBg->setPositionMode(ScreenEntity::POSITION_TOPLEFT);
	addChild(topBarBg);
	
	logo = new ScreenImage("barlogo.png");	
	addChild(logo);		
	
	
	playButton = new UIImageButton("play_button.png");
	addChild(playButton);
	playButton->setPosition(10,8);
	
	resizer = new ScreenImage("corner_resize.png");	
	addChild(resizer);
	resizer->setColor(0,0,0,0.4);
	
	modalBlocker = new ScreenShape(ScreenShape::SHAPE_RECT, 10,10);
	modalBlocker->setColor(0,0,0,0.5);
	modalBlocker->setPositionMode(ScreenEntity::POSITION_TOPLEFT);
	modalBlocker->enabled = false;	
	modalBlocker->blockMouseInput = true;
	addChild(modalBlocker);
	
	newProjectWindow = new NewProjectWindow();
	newProjectWindow->visible = false;
	
	exampleBrowserWindow = new ExampleBrowserWindow();
	exampleBrowserWindow->visible = false;
	
	newFileWindow = new NewFileWindow();
	newFileWindow->visible = false;
	
	textInputPopup = new TextInputPopup();
	textInputPopup->visible = false;
	
	currentEditor = NULL;
	
	isDragging  = false;
	dragLabel = new ScreenLabel("NONE", 11, "sans");
	dragLabel->setPosition(0,-15);
	
	dragEntity = new ScreenEntity();
	dragEntity->addChild(dragLabel);
	addChild(dragEntity);
	dragEntity->visible = false;	
	
	CoreServices::getInstance()->getCore()->getInput()->addEventListener(this, InputEvent::EVENT_MOUSEUP);
	CoreServices::getInstance()->getCore()->getInput()->addEventListener(this, InputEvent::EVENT_MOUSEMOVE);
}

void PolycodeFrame::showModal(UIWindow *modalChild) {
	modalBlocker->enabled = true;
	
	this->modalChild = modalChild;
	addChild(modalChild);
	modalChild->showWindow();
	modalChild->addEventListener(this, UIEvent::CLOSE_EVENT);
	Resize(frameSizeX, frameSizeY);
}

PolycodeProjectBrowser *PolycodeFrame::getProjectBrowser() {
	return projectBrowser;
}

void PolycodeFrame::addEditor(PolycodeEditor *editor) {
	editors.push_back(editor);
	editorHolder->addChild(editor);
	editor->enabled = false;
}

void PolycodeFrame::showEditor(PolycodeEditor *editor) {
	if(currentEditor) {
		currentEditor->enabled = false;
		currentEditor = NULL;
	}
	
	currentEditor = editor;
	currentEditor->enabled = true;
	
	Resize(frameSizeX, frameSizeY);
}

void PolycodeFrame::hideModal() {
	if(modalChild) {
		removeChild(modalChild);
		modalChild->removeEventListener(this, UIEvent::CLOSE_EVENT);	
		modalChild->hideWindow(); 
		modalChild = NULL;
	}
	modalBlocker->enabled = false;		
}

void PolycodeFrame::handleEvent(Event *event) {
	
	if(event->getDispatcher() == CoreServices::getInstance()->getCore()->getInput()) {
		switch(event->getEventCode()) {
			case InputEvent::EVENT_MOUSEUP:
				if(isDragging) {
					if(currentEditor) {
						InputEvent *inputEvent = (InputEvent*) event;						
						Number posX = inputEvent->mousePosition.x - editorHolder->getPosition2D().x;
						Number posY = inputEvent->mousePosition.y - editorHolder->getPosition2D().y;						
						currentEditor->handleDroppedFile(draggedFile, posX, posY);
					}
				}
				isDragging = false;
				dragEntity->visible = false;
			break;
			case InputEvent::EVENT_MOUSEMOVE:
				if(isDragging) {
					dragEntity->setPosition(((InputEvent*)event)->mousePosition);
				}
			break;	
		}
	}

	if(event->getDispatcher() == projectBrowser->treeContainer->getRootNode()) {
		switch (event->getEventCode()) {
			case UITreeEvent::DRAG_START_EVENT:
			{
				UITreeEvent *treeEvent = (UITreeEvent*) event;
				BrowserUserData *data = (BrowserUserData*)treeEvent->selection->getUserData();
				draggedFile = data->fileEntry;
				dragLabel->setText(data->fileEntry.name);
				dragEntity->visible = true;
				isDragging = true;
//				printf("START DRAG: %s\n", data->fileEntry.name.c_str());
			}
			break;
		}
	}

	if(event->getDispatcher() == modalChild) {
		if(event->getEventType() == "UIEvent" && event->getEventCode() == UIEvent::CLOSE_EVENT) {
			hideModal();
		}
	} else {
		if(event->getEventType() == "UIEvent" && event->getEventCode() == UIEvent::CLICK_EVENT && event->getDispatcher() == newProjectButton) {

			newProjectWindow->ResetForm();
			showModal(newProjectWindow);

		}	
		
	if(event->getEventType() == "UIEvent" && event->getEventCode() == UIEvent::CLICK_EVENT && event->getDispatcher() == examplesButton) {

			newProjectWindow->ResetForm();
			showModal(exampleBrowserWindow);

		}			
		
	}
}

void PolycodeFrame::Resize(int x, int y) {	
	
	frameSizeX = x;
	frameSizeY = y;
	
	welcomeEntity->setPosition((x-welcomeImage->getWidth()) / 2,
		(y-welcomeImage->getHeight()) / 2); 
	
	topBarBg->setShapeSize(x, 45);
	logo->setPosition(x-logo->getWidth()-2, 2);	
	resizer->setPosition(x-resizer->getWidth()-1, y-resizer->getHeight()-1);	
	projectBrowser->Resize(200, y-45);
	
	modalBlocker->setShapeSize(x, y);
	
	editorHolder->setPosition(200, 45);
	
	if(currentEditor) {
		currentEditor->Resize(x-200, y-45);
	}
	
	if(this->modalChild) {
		modalChild->setPosition((x-modalChild->getWidth())/2.0f, (y-modalChild->getHeight())/2.0f);
	}
}

PolycodeFrame::~PolycodeFrame() {
	
}

