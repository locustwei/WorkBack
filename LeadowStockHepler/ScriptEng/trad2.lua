--[[
[名称]:摸顶卖出
[说明]:当股票（参数1）冲高回落，跌幅超过（参数2）%，并且股价大于于（参数3）元时卖出。
[Symbol]:股票代码
[nPercent]:回落跌幅（百分比%）
[nValue]:股价高于（元）
[nVolumn]:卖出数（股）
--]]

if Symbol==nil then
	return "Symbol = nil"
end

local p = StockData:new(Symbol)

if p.Hight == 0 then
	return "p.High = 0"
end

local n = (p.Current - p.High) / p.High        

if (p.Current > nValue) and (n > nPercent) then
	return p:Sell(nVolumn)                    
else
	return "条件不成立"
end