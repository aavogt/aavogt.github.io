{-# LANGUAGE CPP, ImplicitParams #-}
import Rapids
import Waterfall.SVG

main = do
  Right king <- readSVG "king.svg"
  let ?king = foldMap toPath king
  write spikes

spikes :: (?king::Path) => Solid
spikes = unitPolygon 3
  & sweepRuled orientKing
  & rotated
      ez (pi/3)
      ez (pi/6)
  & rotate ey (pi/2)

orientKing :: (?king::Path) => V2 Double -> Path
orientKing v = rotate
  ez (unangle v)
  ex (pi/2)
  ?king

write :: Solid -> IO ()
write = writeSTEPColor (takeWhile (/= '.') __FILE__ ++ ".step")
