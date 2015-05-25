------------------------------------------------------------------------------
-- This script just demonstrates how the various bind modifiers can be
-- used and manipulated.
--
-- Step through this script with the included MAX file loaded to see how
-- each bind modifier works.
------------------------------------------------------------------------------


-- Remove any bind modifiers if they're already applied in the sample scene --
(
	Format "Setting up scene... "
	for obj in objects do (
		for i in obj.modifiers.count to 1 by -1 do (
			local c = ClassOf obj.modifiers[i]
			if (c == BindToFace OR
				c == BindToShape OR
				c == BindToNode OR
				c == BindToPoint) then DeleteModifier obj i
		)
	)
	sliderTime = 0f
	Format "Done\n"
)
------------------------------------------------------------------------------


-- Deform with nodes ---------------------------------------------------------
(
	Format "Setting up BindToNode sample... "

	local thisObj = $BoundPlane03
	local nodes = #($DeformerDummy01, $DeformerDummy02)

	-- Radius of influence around nodes, for weighting
	local radius = 120.0

	-- Create and add the modifier
	local m = BindToNode()
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
			bindOps.Bind m thisObj vertIndex nodeIndex w
		)
	)

	Format "Done.\n"
)

-- Deform with shapes ---------------------------------------------------------
(
	Format "Setting up BindToShape sample... "

	local thisObj = $BoundPlane02
	local shapeNode = $DeformerShape

	-- radius of influence around shape, for weighting
	local radius = 75.0

	-- Create and add the modifier
	local m = BindToShape()
	AddModifier thisObj m

	-- Add a shape node (only one in this case, but it has several sub-splines)
	bindOps.AddNode m thisObj shapeNode

	-- Initialize the number of points used by modifier
	bindOps.SetNumPoints m thisObj.numVerts

	-- Loop through all points and bind them to the shapes
	for vertIndex in 1 to thisObj.numVerts do
	(
		local vp = GetVert thisObj vertIndex

		for splineIndex in 1 to (NumSplines shapeNode) do
		(
			-- Get closest point on given spline
			local param = NearestPathParam shapeNode splineIndex vp
			local sp = PathInterp shapeNode splineIndex param
			local d = Distance vp sp

			if (d >= radius) then continue

			-- Smooth weighting falloff
			local w = ((d / radius)) * 2
			w = pow 2.0 (-2.0*w*w)

			-- Bind the vertex
			bindOps.Bind m thisObj vertIndex 1 splineIndex param FALSE w
		)
	)

	Format "Done.\n"
)

-- Deform with points ---------------------------------------------------------
(
	Format "Setting up BindToPoint sample... "

	local thisObj = $BoundShape
	local bindObj = $DeformerTeapot

	-- Simple function to get the closest vertex on a mesh to a given point
	fn GetClosestVert pnt meshObj =
	(
		local closestIdx, dist, closest = 999999999

		for i in 1 to meshObj.numVerts do (
			dist = distance pnt (GetVert meshObj i)
			case of (
				(dist == 0.0): return i
				(dist < closest): (
					closest = dist
					closestIdx = i
				)
			)
		)

		return closestIdx
	)

	-- Create and add the modifier
	local m = BindToPoint()
	AddModifier thisObj m

	-- Assign a bind node to the modifier
	bindOps.AddNode m thisObj bindObj

	-- Initialize the number of points used by modifier
	-- Note a spline has (3*knotCount) number of points, to take the
	-- tangent handles into account
	bindOps.SetNumPoints m ((NumKnots thisObj)*3)

	-- Loop through all the knots and bind them to the
	-- closest vertex on the teapot
	for i in 1 to (NumKnots thisObj) do
	(
		-- Find the closest vert on the teapot
		local p = GetKnotPoint thisObj 1 i
		local closestIndex = GetClosestVert p bindObj

		-- Bind it
		-- Note that knot "i" is referred to as point "(i*3-1)" on the spline
		bindOps.Bind m thisObj (i*3-1) 1 closestIndex 1.0
	)

	Format "Done.\n"
)

-- Deform with faces ---------------------------------------------------------
(
	Format "Setting up BindToFace sample... "

	thisObj = $BoundPlane01
	bindObj = $DeformerPlane01

	-- Create the modifier
	local m = BindToFace()

	-- Assign a bind node to the modifier
	bindOps.AddNode m thisObj bindObj

	-- Initialize the number of points used by modifier
	bindOps.SetNumPoints m thisObj.mesh.numVerts

	-- Loop through all the verts on the boundNode
	for vertIndex in 1 to thisObj.mesh.numVerts do
	(
		-- Make a ray pointing from a vertex on the boundNode towards the deformerNode
		local vertPos = (GetVert thisObj.mesh vertIndex) * thisObj.transform
		local r = Ray vertPos [0,0,-1]

		-- Intersect it to see if it hits a face
		local hitRes = IntersectRayEx bindObj r

		-- If it hit, bind the vertex to the hit face
		if (hitRes != undefined) then (
			bindOps.Bind m thisObj vertIndex 1 hitRes[2] 1.0
		)
	)

	-- Add the modifier
	AddModifier thisObj m

	Format "Done.\n"
)

-- Deform with shape (Relative vs. Absolute comparison) ----------------------
(
	Format "Setting up Relative vs. Absolute BindToShape comparison... "

	local relObj = $RelativePlane
	local absObj = $AbsolutePlane
	local relShape = $RelativeShape
	local absShape = $AbsoluteShape

	-- Create and add the modifiers
	local relMod = BindToShape()
	local absMod = BindToShape()
	AddModifier relObj relMod
	AddModifier absObj absMod

	-- Add a shape node
	bindOps.AddNode relMod relObj relShape
	bindOps.AddNode absMod absObj absShape

	-- Initialize the number of points used by modifier
	bindOps.SetNumPoints relMod relObj.numVerts
	bindOps.SetNumPoints absMod absObj.numVerts

	-- Loop through all points and bind them to the shapes
	-- (assumes objects to be Planes, since it marches through
	-- the rows and columns of the plane verts)
	local numCols = relObj.widthSegs + 1
	local numRows = relObj.lengthSegs
	for yIdx in 0 to numRows do
	(
		for xIdx in 1 to numCols do
		(
			local vertIdx = yIdx * numCols + xIdx
			local param = yIdx / (numRows as float)

			bindOps.Bind relMod relObj vertIdx 1 1 param FALSE 1.0
			bindOps.Bind absMod absObj vertIdx 1 1 param TRUE 1.0
		)
	)

	Format "Done.\n"
)
