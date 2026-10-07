{-# LANGUAGE CPP #-}
import Rapids
import Waterfall.SVG

main = do
  Right s <- readSVG "king.svg"
  writeSTEPColor (takeWhile (/= '.') __FILE__ ++ ".step") $ spikes s

spikes s = unitPolygon 3
  & sweepRuled f
  & rotated
      ez (pi/3)
      ez (pi/6)
  & rotate ey (pi/2)
  where f v = rotate
                ez (unangle v)
                ex (pi/2)
                (foldMap toPath s)
