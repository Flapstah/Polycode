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

#include "PolyUIColorBox.h"
#include "PolyInputEvent.h"
#include "PolyLabel.h"
#include "PolyCoreServices.h"
#include "PolyConfig.h"


using namespace Polycode;

UIColorPicker::UIColorPicker() : UIWindow(L"Pick a color!", 300, 240) {
	closeOnEscape = true;
	
//	topPadding
	Config *conf = CoreServices::getInstance()->getConfig();	
		
		
	String fontName = conf->getStringValue("Polycode", "uiDefaultFontName");
	int fontSize = conf->getNumericValue("Polycode", "uiDefaultFontSize");

	String mainBgImage = conf->getStringValue("Polycode", "uiColorPickerMainBg");		
	String mainFrameImage = conf->getStringValue("Polycode", "uiColorPickerMainFrame");
	String hueFrameImage = conf->getStringValue("Polycode", "uiColorPickerHueFrame");
	String hueSelectorImage = conf->getStringValue("Polycode", "uiColorPickerHueSelector");
	String mainSelectorImage = conf->getStringValue("Polycode", "uiColorPickerMainSelector");

	mainBg = new ScreenImage(mainBgImage);
	mainBg->setPosition(padding, topPadding+padding);
	addChild(mainBg);

	mainFrame = new ScreenImage(mainFrameImage);
	mainFrame->setPosition(padding, topPadding+padding);
	
	alphaSlider = new UIHSlider(0, 1.0, mainFrame->getWidth());
	alphaSlider->setPosition(padding, mainFrame->getHeight() + mainFrame->getPosition().y + 13);
	addChild(alphaSlider);
	alphaSlider->addEventListener(this, UIEvent::CHANGE_EVENT);
	
	mainColorRect = new ScreenShape(ScreenShape::SHAPE_RECT, mainFrame->getWidth(), mainFrame->getHeight());
	mainColorRect->setPositionMode(ScreenEntity::POSITION_TOPLEFT);
	mainColorRect->setPosition(padding+1, topPadding+padding+1);
	addChild(mainColorRect);
	addChild(mainFrame);

	hueFrame = new ScreenImage(hueFrameImage);
	hueFrame->setPosition(mainFrame->getPosition().x + mainFrame->getWidth()+10, topPadding+padding);
	addChild(hueFrame);
	
	hueSelector = new ScreenImage(hueSelectorImage);
	hueSelector->setPositionMode(ScreenEntity::POSITION_CENTER);
	hueSelector->setPosition(hueFrame->getPosition().x + (hueFrame->getWidth()/2.0), hueFrame->getPosition().y);
	addChild(hueSelector);	

	hueSelector->setDragLimits(Polycode::Rectangle(hueSelector->getPosition().x,hueSelector->getPosition().y,0,hueFrame->getHeight()));
				
	mainSelector = new ScreenImage(mainSelectorImage);
	mainSelector->setPositionMode(ScreenEntity::POSITION_CENTER);	
	mainSelector->setPosition(mainFrame->getPosition());
	addChild(mainSelector);	
	
	mainColorRect->getMesh()->useVertexColors = true;
	
	mainColorRect->addEventListener(this, InputEvent::EVENT_MOUSEDOWN);
	mainColorRect->addEventListener(this, InputEvent::EVENT_MOUSEUP);
	mainColorRect->addEventListener(this, InputEvent::EVENT_MOUSEUP_OUTSIDE);
	
	
	hueFrame->addEventListener(this, InputEvent::EVENT_MOUSEDOWN);
	hueFrame->addEventListener(this, InputEvent::EVENT_MOUSEUP);
	hueFrame->addEventListener(this, InputEvent::EVENT_MOUSEUP_OUTSIDE);
	
	
	ScreenLabel *label = new ScreenLabel(L"R:", fontSize, fontName);
	label->setPosition(hueFrame->getPosition().x+hueFrame->getWidth() + 15, topPadding+padding + 3);
	addChild(label);
	
	rTextInput = new UITextInput(false, 40, 12);
	rTextInput->setPosition(hueFrame->getPosition().x+hueFrame->getWidth() + 30, topPadding+padding);
	addChild(rTextInput);
	rTextInput->addEventListener(this, UIEvent::CHANGE_EVENT);

	label = new ScreenLabel(L"G:", fontSize, fontName);
	label->setPosition(hueFrame->getPosition().x+hueFrame->getWidth() + 15, topPadding+padding + 33);
	addChild(label);
	
	gTextInput = new UITextInput(false, 40, 12);
	gTextInput->setPosition(hueFrame->getPosition().x+hueFrame->getWidth() + 30, topPadding+padding + 30);
	addChild(gTextInput);
	gTextInput->addEventListener(this, UIEvent::CHANGE_EVENT);
	
	label = new ScreenLabel(L"B:", fontSize, fontName);
	label->setPosition(hueFrame->getPosition().x+hueFrame->getWidth() + 15, topPadding+padding + 63);
	addChild(label);
	
	bTextInput = new UITextInput(false, 40, 12);
	bTextInput->setPosition(hueFrame->getPosition().x+hueFrame->getWidth() + 30, topPadding+padding + 60);
	addChild(bTextInput);
	bTextInput->addEventListener(this, UIEvent::CHANGE_EVENT);
	
	label = new ScreenLabel(L"A:", fontSize, fontName);
	label->setPosition(hueFrame->getPosition().x+hueFrame->getWidth() + 15, topPadding+padding + 93);
	addChild(label);
	
	aTextInput = new UITextInput(false, 40, 12);
	aTextInput->setPosition(hueFrame->getPosition().x+hueFrame->getWidth() + 30, topPadding+padding + 90);
	addChild(aTextInput);
	aTextInput->addEventListener(this, UIEvent::CHANGE_EVENT);
	
	setHue(0.0);
	setSaturationAndValue(0.0, 0.0);
	
	rTextInput->setNumberOnly(true);
	gTextInput->setNumberOnly(true);
	bTextInput->setNumberOnly(true);
	aTextInput->setNumberOnly(true);
	
	lastHueSelectorPosition = 0;
	
	mainSelector->setDragLimits(Polycode::Rectangle(mainColorRect->getPosition().x,mainColorRect->getPosition().y,mainColorRect->getWidth(), mainColorRect->getHeight()));
				
	colorAlpha = 1.0;
}

void UIColorPicker::onClose() {
	visible = false;
	enabled = false;
}

void UIColorPicker::rebuildFromTextInputs() {
	int	newR = atoi(rTextInput->getText().c_str());
	int	newG = atoi(gTextInput->getText().c_str());
	int	newB = atoi(bTextInput->getText().c_str());
	int	newA = atoi(aTextInput->getText().c_str());			
	
	
	if(newR < 0)
		newR = 0;
	if(newR > 255)
		newR = 255;

	if(newG < 0)
		newG = 0;
	if(newG > 255)
		newG = 255;

	if(newB < 0)
		newB = 0;
	if(newB > 255)
		newB = 255;

	if(newA < 0)
		newA = 0;
	if(newA > 255)
		newA = 255;	
	
	Color newColor;
	newColor.setColorRGBA(newR, newG, newB, newA);

	currentS = newColor.getSaturation();
	currentV = newColor.getValue();
	currentH = newColor.getHue();
	colorAlpha = newColor.a;
	
	updateSelectedColor(false);


}

void UIColorPicker::setPickerColor(Color newColor) {
	currentS = newColor.getSaturation();
	currentV = newColor.getValue();
	currentH = newColor.getHue();
	colorAlpha = newColor.a;
	updateSelectedColor();
}

void UIColorPicker::setSaturationAndValue(Number S, Number V) {
	currentS = S;
	currentV = V;	
	updateSelectedColor();
}

void UIColorPicker::updateSelectedColor(bool updateTextFields) {
	selectedColor.setColorHSV(currentH, currentS, currentV);
	selectedColor.a = colorAlpha;
	
	int newR = floor(selectedColor.r*255.0);
	int newG = floor(selectedColor.g*255.0);
	int newB = floor(selectedColor.b*255.0);
	int newA = floor(selectedColor.a*255.0);
		
	mainColorRect->color.a = colorAlpha;
	
	if(updateTextFields) {
		rTextInput->setText(String::IntToString(newR));
		gTextInput->setText(String::IntToString(newG));
		bTextInput->setText(String::IntToString(newB));
		aTextInput->setText(String::IntToString(newA));
	}
	
	Color hueCol;
	hueCol.setColorHSV(currentH, 1.0, 1.0);
	hueCol.a = colorAlpha;

	mainColorRect->getMesh()->getPolygon(0)->getVertex(0)->vertexColor = Color(1.0,1.0,1.0,colorAlpha);
	mainColorRect->getMesh()->getPolygon(0)->getVertex(1)->vertexColor = hueCol;
	mainColorRect->getMesh()->getPolygon(0)->getVertex(2)->vertexColor = Color(0.0,0.0,0.0,colorAlpha);
	mainColorRect->getMesh()->getPolygon(0)->getVertex(3)->vertexColor = Color(0.0,0.0,0.0,colorAlpha);	
	mainColorRect->getMesh()->arrayDirtyMap[RenderDataArray::COLOR_DATA_ARRAY] = true;				
	
		
	hueSelector->setPositionY(hueFrame->getPosition().y + hueFrame->getHeight() - ((currentH/360.0) * hueFrame->getHeight()));
	lastHueSelectorPosition = hueSelector->getPosition().y;
	
	mainSelector->setPosition(mainColorRect->getPosition().x + (currentS * mainColorRect->getWidth()), mainColorRect->getPosition().y + mainColorRect->getHeight() - (currentV * mainColorRect->getHeight()));					
	lastMainSelectorPosition = mainSelector->getPosition2D();
	
	alphaSlider->setSliderValue(colorAlpha);

	dispatchEvent(new Event(), Event::CHANGE_EVENT);
}

Color UIColorPicker::getSelectedColor() {
	return selectedColor;
}

void UIColorPicker::handleEvent(Event *event) {

	if((event->getDispatcher() == rTextInput || event->getDispatcher() == gTextInput || event->getDispatcher() == bTextInput || event->getDispatcher() == aTextInput) && event->getEventType() == "UIEvent") {
		switch(event->getEventCode()) {
			case UIEvent::CHANGE_EVENT:
			{
				rebuildFromTextInputs();
			}
			break;
		}	
	}

	if(event->getDispatcher() == alphaSlider) {
		switch(event->getEventCode()) {
			case UIEvent::CHANGE_EVENT:
			{
				colorAlpha = alphaSlider->getSliderValue();
				updateSelectedColor();
			}
			break;
		}
	}

	if(event->getDispatcher() == hueFrame) {
		switch(event->getEventCode()) {
			case InputEvent::EVENT_MOUSEDOWN:
			{
				InputEvent *inputEvent = (InputEvent*) event;
				hueSelector->setPositionY(inputEvent->getMousePosition().y);
				hueSelector->startDrag(inputEvent->mousePosition.x-hueSelector->getPosition().x,inputEvent->mousePosition.y-hueSelector->getPosition().y);		
				Number newHue = 360.0 - (((inputEvent->getMousePosition().y-hueFrame->getPosition().y)/((hueFrame->getPosition().y+hueFrame->getHeight())-hueFrame->getPosition().y)) * 360.0f);
				setHue(newHue);
						
			}
			break;
			case InputEvent::EVENT_MOUSEUP:
			case InputEvent::EVENT_MOUSEUP_OUTSIDE:			
			{
				hueSelector->stopDrag();
			}
			break;	
		}
	}



	if(event->getDispatcher() == mainColorRect) {
		switch(event->getEventCode()) {
			case InputEvent::EVENT_MOUSEDOWN:
			{
				InputEvent *inputEvent = (InputEvent*) event;			
				mainSelector->setPosition(inputEvent->getMousePosition().x, inputEvent->getMousePosition().y);			
				mainSelector->startDrag(inputEvent->mousePosition.x-mainSelector->getPosition().x,inputEvent->mousePosition.y-mainSelector->getPosition().y);
				
				Number newV = 1.0 - ((inputEvent->getMousePosition().y-mainColorRect->getPosition().y)/((mainColorRect->getPosition().y+mainColorRect->getHeight())-mainColorRect->getPosition().y));

				Number newS = ((inputEvent->getMousePosition().x-mainColorRect->getPosition().x)/((mainColorRect->getPosition().x+mainColorRect->getWidth())-mainColorRect->getPosition().x));
					
				setSaturationAndValue(newS, newV);
			}
			break;
			case InputEvent::EVENT_MOUSEUP:
			case InputEvent::EVENT_MOUSEUP_OUTSIDE:			
			{
				mainSelector->stopDrag();			
			}
			break;	
		}
	}
	
	UIWindow::handleEvent(event);
}

void UIColorPicker::setHue(Number hueNum) {
	if(hueNum < 0.0)
		hueNum = 0.0;
	if(hueNum >= 360.0)
		hueNum = 359.9999;		
		
	currentH = hueNum;
	updateSelectedColor();
}

UIColorPicker::~UIColorPicker() {

}

void UIColorPicker::Update() {

	if(mainSelector->getPosition2D() != lastMainSelectorPosition) {
		lastMainSelectorPosition = mainSelector->getPosition2D();
		
		Number newPosX = lastMainSelectorPosition.x;
		Number newPosY = lastMainSelectorPosition.y;		
		
		Number newV = 1.0 - ((newPosY-mainColorRect->getPosition().y)/((mainColorRect->getPosition().y+mainColorRect->getHeight())-mainColorRect->getPosition().y));

		Number newS = ((newPosX-mainColorRect->getPosition().x)/((mainColorRect->getPosition().x+mainColorRect->getWidth())-mainColorRect->getPosition().x));
					
		setSaturationAndValue(newS, newV);		

	}
	
	if(hueSelector->getPosition().y != lastHueSelectorPosition) {
		lastHueSelectorPosition = hueSelector->getPosition().y;
		
		Number newPosY = lastHueSelectorPosition;
									
		if(newPosY < hueFrame->getPosition().y)
			newPosY =  hueFrame->getPosition().y;
		if(newPosY > hueFrame->getHeight() +  hueFrame->getPosition().y)
			newPosY = hueFrame->getHeight() + hueFrame->getPosition().y;						
						
		hueSelector->setPositionY(newPosY);
					
		Number newHue = 360.0 - (((newPosY-hueFrame->getPosition().y)/((hueFrame->getPosition().y+hueFrame->getHeight())-hueFrame->getPosition().y)) * 360.0f);
		setHue(newHue);
	}	

	UIWindow::Update();
}

UIColorBox::UIColorBox(Color initialColor, Number width, Number height) : ScreenEntity() {

	Config *conf = CoreServices::getInstance()->getConfig();	

	String frameImageFile = conf->getStringValue("Polycode", "uiColorBoxFrameImage");
	String bgImageFile = conf->getStringValue("Polycode", "uiColorBoxBgImage");
	Number frameInset = conf->getNumericValue("Polycode", "uiFrameInset");

	Number st = conf->getNumericValue("Polycode", "uiColorBoxFrameImageT");
	Number sr = conf->getNumericValue("Polycode", "uiColorBoxFrameImageR");
	Number sb = conf->getNumericValue("Polycode", "uiColorBoxFrameImageB");
	Number sl = conf->getNumericValue("Polycode", "uiColorBoxFrameImageL");


	bgImage = new ScreenShape(ScreenShape::SHAPE_RECT, width-(frameInset*2), height-(frameInset*2));
	bgImage->setPositionMode(ScreenEntity::POSITION_TOPLEFT);
	bgImage->loadTexture(bgImageFile);
	addChild(bgImage);

	bgImage->addEventListener(this, InputEvent::EVENT_MOUSEDOWN);

	colorShape = new ScreenShape(ScreenShape::SHAPE_RECT, width-(frameInset*2), height-(frameInset*2));
	colorShape->setPositionMode(ScreenEntity::POSITION_TOPLEFT);
	addChild(colorShape);

		
	frameImage = new UIBox(frameImageFile, st,sr,sb,sl, width, height);	
	addChild(frameImage);
	
	colorPicker = new UIColorPicker();
	colorPicker->setPosition(width + 20, -colorPicker->getHeight()/2.0);
	
	colorPicker->addEventListener(this, Event::CHANGE_EVENT);
	
	addChild(colorPicker);
	colorPicker->visible = false;
	colorPicker->enabled = false;

	this->width = width;
	this->height = height;
	
	setBoxColor(initialColor);
}

Color UIColorBox::getSelectedColor() {
	return colorPicker->getSelectedColor();
}

UIColorBox::~UIColorBox() {

}

void UIColorBox::setBoxColor(Color newColor) {
	colorPicker->setPickerColor(newColor);
}

void UIColorBox::showColorPicker() {
	colorPicker->visible = true;
	colorPicker->enabled = true;
}
		
void UIColorBox::handleEvent(Event *event) {

	if(event->getDispatcher() == colorPicker) {
		switch(event->getEventCode()) {
			case Event::CHANGE_EVENT:
				colorShape->color = colorPicker->getSelectedColor();
				dispatchEvent(new UIEvent(), UIEvent::CHANGE_EVENT);
			break;
		}		
	}

	if(event->getDispatcher() == bgImage) {
		switch(event->getEventCode()) {
			case InputEvent::EVENT_MOUSEDOWN:
				showColorPicker();
			break;
		}
	}
}
