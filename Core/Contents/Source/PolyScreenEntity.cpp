/*
 Copyright (C) 2011 by Ivan Safrin
 
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

#include "PolyScreenEntity.h"
#include "PolyInputEvent.h"
#include "PolyRectangle.h"
#include "PolyPolygon.h"
#include "PolyVertex.h"
#include "PolyRenderer.h"

inline double round(double x) { return floor(x + 0.5); }

using namespace Polycode;

ScreenEntity::ScreenEntity() : Entity(), EventDispatcher() {
	color = Color(1.0f,1.0f,1.0f,1.0f);
	width = 1;
	height = 1;
	hitwidth = 1;
	hitheight = 1;
	backfaceCulled = false;
	positionMode = POSITION_TOPLEFT;
	mouseOver = false;
	isDragged = false;

	dragOffsetX = 0;
	dragOffsetY = 0;
	parentEntity = NULL;
	zindex = 0;
	
	depthWrite = false;
	depthTest = false;
	
	focusable = false;
	hasFocus = false;
	focusChildren = false;	
	focusedChild = NULL;
	blockMouseInput = false;
	
	snapToPixels = false;

	lastClickTicks = 0;
	dragLimits = NULL;
	
	xmouse = 0;
	ymouse = 0;
	
	processInputEvents = false;
	
}

void ScreenEntity::focusNextChild() {
	int j = 0;
	if(focusedChild) {
		for(int i=0; i < children.size(); i++) {
			if(children[i] == focusedChild)
				j = i;
		}
	}
	
	for(int i=0; i < children.size(); i++) {
		if(((ScreenEntity*)children[j])->isFocusable() && children[j] != focusedChild) {
			focusChild(((ScreenEntity*)children[j]));
			return;
		}
		
		j++;
		if(j == children.size())
			j = 0;
	}
}

Number ScreenEntity::getRotation() const {
	return this->getRoll();
}

void ScreenEntity::focusChild(ScreenEntity *child) {
	if(focusedChild != NULL) {
		focusedChild->onLoseFocus();
		focusedChild->hasFocus = false;
	}
	focusedChild = child;
	focusedChild->hasFocus = true;
	focusedChild->onGainFocus();
}

bool ScreenEntity::isFocusable() const {
	return focusable;
}

void ScreenEntity::startDrag(Number xOffset, Number yOffset) {
	isDragged = true;
	dragOffsetX = xOffset;
	dragOffsetY = yOffset;
}

void ScreenEntity::stopDrag() {
	isDragged = false;
}

ScreenEntity::~ScreenEntity() {

}

void ScreenEntity::setBlendingMode(int newBlendingMode) {
	blendingMode = newBlendingMode;
}

void ScreenEntity::setPosition(Number x, Number y) {
	position.x  = x;
	position.y  = y;
	matrixDirty = true;
}

void ScreenEntity::setPosition(const Vector2 &v) {
	position.x  = v.x;
	position.y  = v.y;
	matrixDirty = true;	
}

void ScreenEntity::setScale(Number x, Number y) {
	scale.x = x;
	scale.y = y;
	matrixDirty = true;	
}

void ScreenEntity::setScale(const Vector2 &v) {
	scale.x = v.x;
	scale.y = v.y;
	matrixDirty = true;	
}


Number ScreenEntity::getWidth() const {
	return width;
}

Number ScreenEntity::getHeight() const {
	return height;
}

bool isPointInsidePolygon2D(Polycode::Polygon *poly, const Vector2 &p) {
	Number angle = 0.0;
	Vector2 p1,p2;
/*	
	printf("PIP: %f,%f in [%f,%f], [%f,%f], [%f,%f], [%f,%f]\n", p.x, p.y, 
		poly->getVertex(0)->x, poly->getVertex(0)->y,
		poly->getVertex(1)->x, poly->getVertex(1)->y,
		poly->getVertex(2)->x, poly->getVertex(2)->y,
		poly->getVertex(3)->x, poly->getVertex(3)->y);
*/
	for (int i=0; i < poly->getVertexCount(); i++) {

		p1.x = poly->getVertex(i)->x - p.x;
		p1.y = poly->getVertex(i)->y - p.y;
		p2.x = poly->getVertex((i+1)%poly->getVertexCount())->x - p.x;
		p2.y = poly->getVertex((i+1)%poly->getVertexCount())->y - p.y;
		
		Vector2 vec(p1.x,p1.y);
		angle += vec.angle(Vector2(p2.x,p2.y));

	}

	if (fabs(angle) < PI)
		return false;
	else
		return true;
}

bool ScreenEntity::hitTest(const Number x, const Number y) const {

	Vector3 v;	
	Polygon testPoly;
	
	Matrix4 transformMatrix = getConcatenatedMatrix();
	
	v = Vector3(-hitwidth/2.0, -hitheight/2.0,0);
	v = transformMatrix * v;	
	testPoly.addVertex(v.x, v.y, 0.0);
	
	v = Vector3(hitwidth/2.0, -hitheight/2.0,0);
	v = transformMatrix * v;	
	testPoly.addVertex(v.x, v.y, 0.0);

	v = Vector3(hitwidth/2.0, hitheight/2.0,0);
	v = transformMatrix * v;	
	testPoly.addVertex(v.x, v.y, 0.0);

	v = Vector3(-hitwidth/2.0,hitheight/2.0,0);
	v = transformMatrix * v;	
	testPoly.addVertex(v.x, v.y, 0.0);
		
	return isPointInsidePolygon2D(&testPoly, Vector2(x,y));
	
}

void ScreenEntity::setPositionMode(int newPositionMode) {
	positionMode = newPositionMode;
}

void ScreenEntity::_onKeyDown(PolyKEY key, wchar_t charCode) {
	onKeyDown(key, charCode);
	for(int i=0;i<children.size();i++) {
		((ScreenEntity*)children[i])->_onKeyDown(key, charCode);
	}
}

void ScreenEntity::_onKeyUp(PolyKEY key, wchar_t charCode) {
	onKeyUp(key, charCode);
	for(int i=0;i<children.size();i++) {
		((ScreenEntity*)children[i])->_onKeyUp(key, charCode);
	}
}

void ScreenEntity::setDragLimits(Polycode::Rectangle rect) {
	if(!dragLimits)
		dragLimits = new Polycode::Rectangle();
	dragLimits->x = rect.x;
	dragLimits->y = rect.y;
	dragLimits->w = rect.w;
	dragLimits->h = rect.h;		
}

void ScreenEntity::clearDragLimits() {
	delete dragLimits;
	dragLimits = NULL;
}

void ScreenEntity::_onMouseMove(Number x, Number y, int timestamp) {

	if(isDragged) {
		setPosition(x-dragOffsetX,y-dragOffsetY);
		if(dragLimits) {
			if(position.x < dragLimits->x)
				position.x = dragLimits->x;
			if(position.x > dragLimits->x + dragLimits->w)
				position.x = dragLimits->x + dragLimits->w;
			if(position.y < dragLimits->y)
				position.y = dragLimits->y;
			if(position.y > dragLimits->y + dragLimits->h)
				position.y = dragLimits->y + dragLimits->h;
		}
	}
	
	xmouse = x;
	ymouse = y;

	onMouseMove(x,y);
	if(processInputEvents && enabled) {
		if(hitTest(x,y)) {
			dispatchEvent(new InputEvent(Vector2(x,y), timestamp), InputEvent::EVENT_MOUSEMOVE);
			if(!mouseOver) {
				dispatchEvent(new InputEvent(Vector2(x,y), timestamp), InputEvent::EVENT_MOUSEOVER);
				mouseOver = true;
			}
		} else {
			if(mouseOver) {
				dispatchEvent(new InputEvent(Vector2(x,y), timestamp), InputEvent::EVENT_MOUSEOUT);
				mouseOver = false;
			}
		}
	}
	
	if(enabled) {
		for(int i=0;i<children.size();i++) {
			((ScreenEntity*)children[i])->_onMouseMove(x,y, timestamp);
		}
	}
}

bool ScreenEntity::_onMouseUp(Number x, Number y, int mouseButton, int timestamp) {
	bool retVal = false;
	
	if(processInputEvents && enabled) {	
	if(hitTest(x,y)) {
	
		Vector3 localCoordinate = Vector3(x,y,0);
		
		Matrix4 inverse = getConcatenatedMatrix().inverse();
		localCoordinate = inverse * localCoordinate;
		if(positionMode == POSITION_TOPLEFT)
			localCoordinate.x += hitwidth/2.0;
		if(positionMode == POSITION_TOPLEFT)
			localCoordinate.y += hitheight/2.0;
	
		onMouseUp(localCoordinate.x,localCoordinate.y);
		
		InputEvent *inputEvent = new InputEvent(Vector2(localCoordinate.x,localCoordinate.y), timestamp);		
		inputEvent->mouseButton = mouseButton;		
		dispatchEvent(inputEvent, InputEvent::EVENT_MOUSEUP);
		retVal = true;		
	} else {
		
		Vector3 localCoordinate = Vector3(x,y,0);
		
		Matrix4 inverse = getConcatenatedMatrix().inverse();
		localCoordinate = inverse * localCoordinate;
		if(positionMode == POSITION_TOPLEFT)
			localCoordinate.x += hitwidth/2.0;
		if(positionMode == POSITION_TOPLEFT)
			localCoordinate.y += hitheight/2.0;
	
		
		InputEvent *inputEvent = new InputEvent(Vector2(localCoordinate.x,localCoordinate.y), timestamp);		
		inputEvent->mouseButton = mouseButton;
		
		dispatchEvent(inputEvent, InputEvent::EVENT_MOUSEUP_OUTSIDE);
	}
	}
	if(enabled) {
		Vector2 adjust;
		if(positionMode == POSITION_TOPLEFT)
			adjust = Vector2(width/2.0, height/2.0);
	
		for(int i=0;i<children.size();i++) {
			((ScreenEntity*)children[i])->_onMouseUp(x+adjust.x,y+adjust.y, mouseButton, timestamp);
		}
	}
	return retVal;
}

void ScreenEntity::_onMouseWheelUp(Number x, Number y, int timestamp) {
	bool doTest = true;
	
	if(hasMask) {
		if(!((ScreenEntity*)maskEntity)->hitTest(x,y)) {
			doTest = false;
		}	
	}
	
	if(doTest) {
		if(hitTest(x,y) && enabled) {
			onMouseWheelUp(x,y);
			dispatchEvent(new InputEvent(Vector2(x,y), timestamp), InputEvent::EVENT_MOUSEWHEEL_UP);
		}
		if(enabled) {
			for(int i=children.size()-1;i>=0;i--) {				
				((ScreenEntity*)children[i])->_onMouseWheelUp(x,y, timestamp);
				if(((ScreenEntity*)children[i])->blockMouseInput && ((ScreenEntity*)children[i])->enabled) {
					if(((ScreenEntity*)children[i])->hitTest(x,y))
						break;
				}
			}
		}
	}	
}

void ScreenEntity::_onMouseWheelDown(Number x, Number y, int timestamp) {
	bool doTest = true;
	
	if(hasMask) {
		if(!((ScreenEntity*)maskEntity)->hitTest(x,y)) {
			doTest = false;
		}	
	}
	
	if(doTest) {
		if(hitTest(x,y) && enabled) {
			onMouseWheelDown(x,y);
			dispatchEvent(new InputEvent(Vector2(x,y), timestamp), InputEvent::EVENT_MOUSEWHEEL_DOWN);
		}
		if(enabled) {
			for(int i=children.size()-1;i>=0;i--) {				
				((ScreenEntity*)children[i])->_onMouseWheelDown(x,y, timestamp);
				if(((ScreenEntity*)children[i])->blockMouseInput && ((ScreenEntity*)children[i])->enabled) {
					if(((ScreenEntity*)children[i])->hitTest(x,y))
						break;
				}
			}
		}
	}	
}


bool ScreenEntity::_onMouseDown(Number x, Number y, int mouseButton, int timestamp, Vector2 parentAdjust) {
	bool retVal = false;
	
	bool doTest = true;	
	
	if(hasMask) {
		if(!((ScreenEntity*)maskEntity)->hitTest(x+parentAdjust.x,y+parentAdjust.y)) {
			doTest = false;
		}	
	}
	
	if(doTest) {
	if(processInputEvents && enabled) {
	if(hitTest(x+parentAdjust.x,y+parentAdjust.y)) {
		Vector3 localCoordinate = Vector3(x+(parentAdjust.x*2.0),y+(parentAdjust.y*2.0),0);
		
		Matrix4 inverse = getConcatenatedMatrix().inverse();
		localCoordinate = inverse * localCoordinate;
		if(positionMode == POSITION_TOPLEFT)
			localCoordinate.x += hitwidth/2.0;
		if(positionMode == POSITION_TOPLEFT)
			localCoordinate.y += hitheight/2.0;

		
		onMouseDown(localCoordinate.x,localCoordinate.y);
		
		InputEvent *inputEvent = new InputEvent(Vector2(localCoordinate.x,localCoordinate.y)-parentAdjust, timestamp);
		
		inputEvent->mouseButton = mouseButton;
		dispatchEvent(inputEvent, InputEvent::EVENT_MOUSEDOWN);
		
		if(timestamp - lastClickTicks < 400) {
			InputEvent *inputEvent = new InputEvent(Vector2(x,y), timestamp);
			inputEvent->mouseButton = mouseButton;			
			dispatchEvent(inputEvent, InputEvent::EVENT_DOUBLECLICK);
		}
		lastClickTicks = timestamp;		
		retVal = true;
	}
	}
	if(enabled) {
		for(int i=children.size()-1;i>=0;i--) {			
			Vector2 adjust = parentAdjust;
			if(positionMode == POSITION_TOPLEFT)
				adjust += Vector2(width/2.0, height/2.0);
			((ScreenEntity*)children[i])->_onMouseDown(x,y, mouseButton, timestamp, adjust);
			if(((ScreenEntity*)children[i])->blockMouseInput && ((ScreenEntity*)children[i])->enabled) {
				if(((ScreenEntity*)children[i])->hitTest(x,y))
				   break;
			}
		}
	}
	}		
	
	return retVal;
}

void ScreenEntity::setRotation(Number rotation) {
	setRoll(rotation);
}

Vector2 ScreenEntity::getPosition2D() const {
	return Vector2(position.x, position.y);
}

Matrix4 ScreenEntity::buildPositionMatrix() {
	Matrix4 posMatrix;
	switch(positionMode) {
		case POSITION_TOPLEFT:
			posMatrix.m[3][0] = (position.x+floor(width/2.0f)*scale.x)*matrixAdj;
			posMatrix.m[3][1] = (position.y+floor(height/2.0f)*scale.y)*matrixAdj;
			posMatrix.m[3][2] = position.z*matrixAdj;			
		break;
		case POSITION_CENTER:
			posMatrix.m[3][0] = position.x*matrixAdj;
			posMatrix.m[3][1] = position.y*matrixAdj;
			posMatrix.m[3][2] = position.z*matrixAdj;
		break;
	}	
	
	
	if(snapToPixels) {
		posMatrix.m[3][0] = round(posMatrix.m[3][0]);
		posMatrix.m[3][1] = round(posMatrix.m[3][1]);
		posMatrix.m[3][2] = round(posMatrix.m[3][2]);		
	}
	
	return posMatrix;
}

void ScreenEntity::adjustMatrixForChildren() {
	if(positionMode == POSITION_TOPLEFT)
		renderer->translate2D(-floor(width/2.0f), -floor(height/2.0f));	
}