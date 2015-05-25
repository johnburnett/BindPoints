-- Prolly broken in R4
--m=ConvertSkinToBind $ $.modifiers[1]
--AddModifier $ m
--$.modifiers[2].enabled = FALSE
fn ConvertSkinToBind obj skinMod =
(
	-- Find index of skinMod on object
--	local skinIndex = 0
--	for i in 1 to obj.modifiers.count do
--		if (obj.modifiers[i] == skinMod) then skinIndex = i
--
--	if (skinIndex == 0) then return undefined
--	Bfd.UI.PushCommandPanelTaskMode #modify
	local skinIndex = modPanel.GetModifierIndex obj skinMod
--	modPanel.SetCurrentObject skinMod -- ick ick ick

	-- Gather the bones
	local boneObjs = #()
	for i in 1 to (skinOps.GetNumberBones skinMod) do
	(
		local str = skinOps.GetBoneName skinMod i 0
		local obj = try (Execute ("$'" + str + "'")) catch (undefined)
		if (obj == undefined) then
			return undefined
		else
			Append boneObjs obj
	)

	local numPoints = skinOps.GetNumberVertices skinMod

	-- Init Bind mod
	local bindMod = BindToNode()
	bindOps.SetNumPoints bindMod numPoints
	for boneObj in boneObjs do
		bindOps.AddNode bindMod obj boneObj

	-- Copy weight info
	for pIdx in 1 to numPoints do
	(
		local numWeights = skinOps.GetVertexWeightCount skinMod pIdx
		for bIdx in 1 to numWeights do
		(
			local boneIndex = skinOps.GetVertexWeightBoneID skinMod pIdx bIdx
			boneIndex += 1 -- Max3.1, boneIndex is 0 based
			local w = skinOps.GetVertexWeight skinMod pIdx bIdx

			bindOps.Bind bindMod obj pIdx boneIndex w
		)
	)

--	Bfd.UI.PopCommandPanelTaskMode()
	bindMod
)
