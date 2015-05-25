sv=$SF_Plane01_01
emitter=$Plane01
deform=$Plane02
m=BindToPoint()

AddModifier sv m
bindops.addnode m sv deform
bindops.setnumpoints m (emitter.numVerts*2)

pIdx = 1
fIdx = 1
case (classof m) of (
	bindtoface:		bindops.bind m sv pIdx 1 fIdx 1.0
	bindtonode:		bindops.bind m sv pIdx 1 1.0
	bindtopoint:	bindops.bind m sv pIdx 1 fIdx 1.0
	bindtoshape:	bindops.bind m sv pIdx 1 fIdx 0 1.0
)
