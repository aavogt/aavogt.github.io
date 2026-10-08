{-# LANGUAGE CPP, TemplateHaskell #-}
import Rapids
import Data.List

unitCylinder_ = pad 1 $ unitPolygon 19
s = rotatedDeg ex 90 unitCylinder
t = rotatedDeg ex 90 unitCylinder_

-- https://github.com/aavogt/rapids/issues/1
main =
  writeSTEPColor (takeWhile (/= '.') __FILE__ ++ ".step") $ foldr1 (stacked ex) $ intersperse unitGap
      [$purple (hull s), $red s, $blue (hull t), $green t]
