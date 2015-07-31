--[[
股票取数脚本。
调用C++取数接口获取股票数据
--]]

StockData = {}

MARK_SZ = 0  --深市
MARK_SH = 1  --沪市

DATA_ID_SIMPLE = 100

TRAD_ID_BUY = 200
TRAD_ID_SELL = 201

function StockData:new(szSymbol)
	local obj = {Mark=0, Code=szSymbol, Close=0,Open=0,High=0,Low=0,Current=0,Volume=0,Amount=0,Rise=0}
	if string.sub(szSymbol, 0, 1) == '6' then
		obj.Mark = MARK_SH
	else
		obj.Mark = MARK_SZ
	end

    self.__index = self
    local ret = setmetatable(obj, self)
	ret:RefreshData()
	return ret
end


function StockData:RefreshData()
	local fClose, fOpen, fHigh, fLow, fCurrent, dwVolume, fAmount = CallDataFunc(DATA_ID_SIMPLE, self.Mark, self.Code)
	self.Close = fClose
	self.Open = fOpen
	self.High = fHigh
	self.Low = fLow
	self.Current = fCurrent
	self.Volume = dwVolume
	self.Amount = Amount
	if self.Cose == 0 then
		self.Rise = 0
	else
		self.Rise = ((self.Current - self.Close) / self.Close)*100
	end
end


function StockData:Buy(nVolumn)
	return CallTradFunc(TRAD_ID_BUY, self.Mark, self.Code, nVolumn)
end

function StockData:Sell(nVolumn)
	return CallTradFunc(TRAD_ID_SELL, self.Mark, self.Code, nVolumn)
end