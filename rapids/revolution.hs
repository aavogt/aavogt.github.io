{-# LANGUAGE CPP, TemplateHaskell #-}
import Rapids
import Waterfall.SVG

main = do
  Right s <- readSVG "king.svg"
  writeSTEPColor (takeWhile (/= '.') __FILE__ ++ ".step") $
    rotate ey (pi/2) $ $red $ revolution s

