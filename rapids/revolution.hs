{-# LANGUAGE CPP, TemplateHaskell #-}
import Rapids
import Waterfall.SVG

main = do
  Right s <- readSVG "king.svg"
  write $ rotate ey (pi/2) $ $red $ revolution s

write :: Solid -> IO ()
write = writeSTEPColor (takeWhile (/= '.') __FILE__ ++ ".step")

