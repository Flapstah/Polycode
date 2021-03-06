class "PhysicsJoint"







function PhysicsJoint:PhysicsJoint(...)
	for k,v in pairs(arg) do
		if type(v) == "table" then
			if v.__ptr ~= nil then
				arg[k] = v.__ptr
			end
		end
	end
	if self.__ptr == nil and arg[1] ~= "__skip_ptr__" then
		self.__ptr = Physics2D.PhysicsJoint(unpack(arg))
		Polycore.__ptr_lookup[self.__ptr] = self
	end
end



function PhysicsJoint:__delete()
	Polycore.__ptr_lookup[self.__ptr] = nil
	Physics2D.delete_PhysicsJoint(self.__ptr)
end
