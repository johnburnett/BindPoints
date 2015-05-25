-- Test out the bind modifiers... bindTypes is an array of the bind modifiers to be tested
-- Uses BindTest.max scene file....

--bindTypes = #(BindToFace)
--bindTypes = #(BindToNode)
--bindTypes = #(BindToPoint)
--bindTypes = #(BindToShape)
bindTypes = #(BindToFace, BindToNode, BindToPoint, BindToShape)

for bind in bindTypes do
(
	local thisObj = case bind of (
		BindToFace: $FaceObj
		BindToNode: $NodeObj
		BindToPoint: $PointObj
		BindToShape: $ShapeObj
	)
	local nodes = case bind of (
		BindToFace: #($Mesh01, $Mesh02)
		BindToNode: #($Mesh03, $Mesh04)
		BindToPoint: #($Mesh05, $Mesh06)
		BindToShape: #($Shape01, $Shape02)
	)

	-- Radius of influence around nodes, for weighting
	local radius = 120.0

	-- Create and add the modifier
	local m = bind()
	AddModifier thisObj m

	-- Add a couple nodes
	for obj in nodes do bindOps.AddNode m thisObj obj

	-- Initialize the number of points used by modifier
	bindOps.SetNumPoints m thisObj.numVerts

	-- Loop through all points and bind them to the nodes
	for vertIndex in 1 to thisObj.numVerts do
	(
		local vp = GetVert thisObj vertIndex

		for nodeIndex in 1 to nodes.count do
		(
			-- Get distance from vertex to node
			local d = Distance vp nodes[nodeIndex].pos

			-- Check if vertex falls within node's influence
			if (d >= radius) then continue

			-- Smooth weighting falloff
			local w = ((d / radius)) * 2
			w = pow 2.0 (-2.0*w*w)

			-- Bind the vertex
			case bind of (
				BindToFace: bindOps.Bind m thisObj vertIndex nodeIndex 1 w
				BindToNode: bindOps.Bind m thisObj vertIndex nodeIndex w
				BindToPoint: bindOps.Bind m thisObj vertIndex nodeIndex 1 w
				BindToShape: bindOps.Bind m thisObj vertIndex nodeIndex 1 0.0 TRUE w
			)
		)
	)
)
