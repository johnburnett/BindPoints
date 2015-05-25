debug = false
trackVert = 11
sv = undefined
boneShapes = #()

fn DoIt m = (
	m.RemoveBindNode 1
)

fn DoIt m = (
	m.thisObj = (refs.dependents m)[1]
	m.InitBinds()

	m.AddBindNode $Line05
	m.AddBindNode $Line08

	local numVerts = m.thisObj.numVerts as float
	for i in 2 to numVerts do
	(
		--local p = (i-1)/(numVerts-1)
		local p = GetVert m.thisObj i
		local param = NearestPathParam (m.GetBindNode 1) 1 p
		param = PathToLengthParam (m.GetBindNode 1) 1 param
		m.Bind i 1 1 param 0.5
		m.Bind i 2 1 param 0.5
	)
)

if false then (
	sv = $SH_Head_01
	boneShapes = #($SH_Head_01_SimSpline01,$SH_Head_01_SimSpline02,$SH_Head_01_SimSpline03,$SH_Head_01_SimSpline04,$SH_Head_01_SimSpline05,$SH_Head_01_SimSpline06,$SH_Head_01_SimSpline07)
	AddModifier sv (BindToShape())
	select sv
	SetCommandPanelTaskMode mode:#modify
)

fn NumHairs sv = (
	--sv.numVerts / (sv.shag.knots+1)
	48
)

fn NumVertsPerHair sv = (
	--sv.shag.knots + 1
	21
)

fn GetHairVertIndex sv hairIndex hairVertIndex = (
	(NumVertsPerHair sv) * (hairIndex-1) + hairVertIndex
)

fn GetHairIndex sv vertIndex = (
	(vertIndex-1) / (NumVertsPerHair sv) + 1
)

fn SetHairBone m sv hairIndex boneIndex perc = (
	format "hairIndex: %, boneIndex: %, bone: %, perc: %\n" hairIndex boneIndex m.shapeNodes[boneIndex].name perc

	-- root vert always gets weight of 0.0 to anchor it,

	local numVerts = NumVertsPerHair sv
	local sIdx = GetHairVertIndex sv hairIndex 2
	local eIdx = GetHairVertIndex sv hairIndex numVerts
	local d = 1.0 / (numVerts-2)
	local curD = 0.0
	-- assign weights to verts 2 through VertsPerHair
	for i in sIdx to eIdx do (
		--Linear weight
		local w = perc --curD * perc
		--Gauss weight
		--local x = (1.0 - p) * 2
		--local w = (pow 2.0 (-2.0*x*x)) * perc

if debug then format "vIdx: %, w: %\n" i w
		m.Bind i boneIndex 1 curD w
		curD += d
	)
)

fn DoIt m = (
	m.thisObj = (refs.dependents m)[1]
	m.InitBinds()
	for bs in boneShapes do m.AddBindNode bs

	local rootArray = Bfd.Spline.GetRoots boneShapes

	format "Assigning Bones...\n"
	for hIdx in 1 to (NumHairs sv) do (
		local rootIdx = GetHairVertIndex sv hIdx 2
		local rootPos = (GetVert sv.mesh rootIdx) * sv.transform
		local res = Bfd.Math.GetClosestPoints rootPos rootArray
		local idxArray = res[1]
		local distArray = res[2]
-- TODO: support for ONE SimSpline
		local boneIndexA = idxArray[1]
		local boneIndexB = idxArray[2]
		local percB = distArray[1] / (distArray[1] + distArray[2])
		local percA = 1.0 - percB

		SetHairBone m sv hIdx boneIndexA percA
		SetHairBone m sv hIdx boneIndexB percB
	)
)

plugin SimpleMod BindToShape
	name:"BindToShape"
	classID:#(0x89eddf75, 0xc71264c6)
	version:1
(
	struct _bind (
		shapeIndex,
		splineIndex,
		lengthParam,
		basePos,
		weight)

	local thisObj = undefined
	local shapeNodes = #()
	local vertexBinds = #()

	-- Wipe vertexBinds, init vertexBinds array.count == numverts
	fn InitBinds =
	(
		if (Bfd.Node.Exists thisObj) then (
			vertexBinds = for i in 1 to thisObj.numVerts collect #()
		)
	)

	-- Remove any binds that have indexes higher than the number of splines in a shape
	fn CheckSplineIndexes =
	(
		for vIdx in 1 to vertexBinds.count do
		(
			for bIdx in 1 to vertexBinds[vIdx].count do
			(
				local cnt = NumSplines shapeNodes[vertexBinds[vIdx][bIdx]]
				if (vertexBinds[vIdx][bIdx].splineIndex > cnt) then UnBind vIdx bIdx
			)
		)
	)

	fn GetBind vIdx bIdx =
	(
		if (vIdx > 0 AND vIdx <= vertexBinds.count AND
			bIdx > 0 AND bIdx <= vertexBinds[vIdx].count) then
		(
			return vertexBinds[vIdx][bIdx]
		) else
			return undefined
	)

	fn GetNumBinds vIdx =
	(
		if (vIdx > 0 AND vIdx <= vertexBinds.count) then
			return vertexBinds[vIdx].count
		else
			return 0
	)

	fn UnBind vIdx bIdx =
	(
		if (vIdx > 0 AND vIdx <= vertexBinds.count AND
			bIdx > 0 AND bIdx <= vertexBinds[vIdx].count) then
		(
			DeleteItem vertexBinds[vIdx] bIdx
			return TRUE
		) else
			return FALSE
	)

	fn Bind vIdx shapeIndex splineIndex lengthParam weight =
	(
		if (shapeIndex < 0 OR shapeIndex > shapeNodes.count) then return FALSE

		local p = LengthInterp shapeNodes[shapeIndex] splineIndex lengthParam

		local b = _bind	shapeIndex:shapeIndex		\
						splineIndex:splineIndex		\
						lengthParam:lengthParam		\
						basePos:p					\
						weight:weight

if debug then format "Bind - vIdx: %, shp: %, spl: %, len: %, w: %, basePos: %\n" vIdx shapeIndex splineIndex lengthParam weight p

		append vertexBinds[vIdx] b

		return TRUE
	)

	fn AddBindNode sNode =
	(
		if (FindItem shapeNodes sNode) == 0 then
		(
			Append shapeNodes sNode
			return TRUE
		) else
			return FALSE
	)

	fn RemoveBindNode shapeIndex =
	(
		if (shapeIndex < 1 OR shapeIndex > shapeNodes.count) then return FALSE

		DeleteItem shapeNodes shapeIndex

		-- remove all references to the node, and decrement any
		-- bind indexes in the array that were "above" the node index
		for vIdx in 1 to vertexBinds.count do (
			for bIdx in vertexBinds[vIdx].count to 1 by -1 do (
				if (vertexBinds[vIdx][bIdx].shapeIndex == shapeIndex) then (
					format "Unbind: v: %, b: %, shp: %\n" vIdx bIdx shapeIndex
					UnBind vIdx bIdx
				) else if (vertexBinds[vIdx][bIdx].shapeIndex > shapeIndex) then (
					format "Decrement: v: %, b: %, shp: %\n" vIdx bIdx shapeIndex
					vertexBinds[vIdx][bIdx].shapeIndex = vertexBinds[vIdx][bIdx].shapeIndex - 1
				)
			)
		)

		return TRUE
	)

	fn GetNumBindNodes =
	(
		return shapeNodes.count
	)

	fn GetBindNode shapeIndex =
	(
		if (shapeIndex > 0 AND shapeIndex <= shapeNodes.count) then
			return shapeNodes[shapeIndex]
		else
			return undefined
	)

	rollout DLGbindRollout "BindToShape Parameters"
	(
		button DLGdoBind "Bind"

		on DLGdoBind pressed do
		(
			DoIt this
		)
	)

	on map vIdx p do
	(
		if (vIdx < 1 OR vIdx > vertexBinds.count) then return p

if debug then format "t: %, vIdx: %\n" currentTime vIdx
		try (

--			local offset = [0,0,0]
			for bIdx in 1 to vertexBinds[vIdx].count do (
				local b = vertexBinds[vIdx][bIdx]
				local newPos = if ((b.lengthParam-1.0) > -0.000001) then -- workaround for LengthInterp giving bad values at spline ends
							PathInterp shapeNodes[b.shapeIndex] b.splineIndex b.lengthParam
						else
							LengthInterp shapeNodes[b.shapeIndex] b.splineIndex b.lengthParam
				local offset = (newPos-b.basePos) * b.weight
				p = p + offset
if debug AND vIdx == trackVert then
	format "bIdx: % shp: % spl: % len: % w: % newPos: % offset: %\n" bIdx b.shapeIndex b.splineIndex b.lengthParam b.weight newPos offset
			)
		) catch (
			format "Fail: %\n" vIdx
		)
		return p
	)
)
