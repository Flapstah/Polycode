require "Polycode/EventDispatcher"

class "ScreenManager" (EventDispatcher)







function ScreenManager:ScreenManager(...)
	if type(arg[1]) == "table" and count(arg) == 1 then
		if ""..arg[1]:class() == "EventDispatcher" then
			self.__ptr = arg[1].__ptr
			return
		end
	end
	for k,v in pairs(arg) do
		if type(v) == "table" then
			if v.__ptr ~= nil then
				arg[k] = v.__ptr
			end
		end
	end
	if self.__ptr == nil and arg[1] ~= "__skip_ptr__" then
		self.__ptr = Polycore.ScreenManager(unpack(arg))
		Polycore.__ptr_lookup[self.__ptr] = self
	end
end

function ScreenManager:removeScreen(screen)
	local retVal = Polycore.ScreenManager_removeScreen(self.__ptr, screen.__ptr)
end

function ScreenManager:addScreen(screen)
	local retVal = Polycore.ScreenManager_addScreen(self.__ptr, screen.__ptr)
end

function ScreenManager:Update()
	local retVal =  Polycore.ScreenManager_Update(self.__ptr)
end



function ScreenManager:__delete()
	Polycore.__ptr_lookup[self.__ptr] = nil
	Polycore.delete_ScreenManager(self.__ptr)
end
