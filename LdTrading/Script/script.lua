MARK_SZ = 0
MARK_SH = 1

FUN_ID_SIMPLE = 100

function getCurrent(nMark, szCode)
	local fClose, fOpen, fHigh, fLow, fCurrent, dwVolume, fAmount = CallDataFunc(FUN_ID_SIMPLE, nMark, szCode)
	return fCurrent
end

function getOpen(nMark, szCode)
	local fClose, fOpen, fHigh, fLow, fCurrent, dwVolume, fAmount = CallDataFunc(FUN_ID_SIMPLE, nMark, szCode)
	return fOpen
end

function getHigh(nMark, szCode)
	local fClose, fOpen, fHigh, fLow, fCurrent, dwVolume, fAmount = CallDataFunc(FUN_ID_SIMPLE, nMark, szCode)
	return fHigh
end
		
function getLow(nMark, szCode)
	local fClose, fOpen, fHigh, fLow, fCurrent, dwVolume, fAmount = CallDataFunc(FUN_ID_SIMPLE, nMark, szCode)
	return fLow
end

function getClose(nMark, szCode)
	local fClose, fOpen, fHigh, fLow, fCurrent, dwVolume, fAmount = CallDataFunc(FUN_ID_SIMPLE, nMark, szCode)
	return fClose
end

function getVolumn(nMark, szCode)
	local fClose, fOpen, fHigh, fLow, fCurrent, dwVolume, fAmount = CallDataFunc(FUN_ID_SIMPLE, nMark, szCode)
	return dwVolume
end

function ChangePercent(nMark, szCode)
	local fClose, fOpen, fHigh, fLow, fCurrent, dwVolume, fAmount = CallDataFunc(FUN_ID_SIMPLE, nMark, szCode)

	return ((fCurrent - fClose) / fClose)*100
end

