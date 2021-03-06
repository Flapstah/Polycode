screen = Screen()

sun = ScreenShape(SHAPE_CIRCLE, 100,100, 30)
sun:setPosition(640/2, 480/2)
sun:setColor(1.0, 0.0, 0, 1)
sun.colorAffectsChildren = false
screen:addChild(sun)

planet = ScreenShape(SHAPE_CIRCLE, 50,50, 30)
planet:setPosition(150,0)
planet:setColor(0.2, 0.8, 0, 1)
planet.colorAffectsChildren = false
sun:addChild(planet)

moon = ScreenShape(SHAPE_CIRCLE, 20,20, 30)
moon:setPosition(50,0)
moon:setColor(1, 1, 0.6, 1)
planet:addChild(moon)

planetRotation = 0
moonRotation = 0

function Update(elapsed)
        planetRotation = planetRotation + elapsed
        moonRotation = moonRotation + (elapsed * 6)
        planet:setPosition(cos(planetRotation)*150, sin(planetRotation)*150)
        moon:setPosition(cos(moonRotation)*50, sin(moonRotation)*50)
end
