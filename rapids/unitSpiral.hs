{-# LANGUAGE CPP, TemplateHaskell #-}
import Data.List
import Rapids

main = write [
  $green $ unitSpiral 2 $ rectangle 0.5 0.2,
  $red $ offset 0.05 [5, 6] $ mirrorZ1 taperedSpiral,
  fan ]

fan :: Solid
fan = $blue
  blades
    + $yellow (scale 0.5 0.3 unitCylinder)
    - $brown (scale 0.3 1 unitCylinder)

polar :: Int -> Solid -> Solid
polar n s = unions [ rotate ez th s
  | j <- [0 .. n - 1],
    let th = 2 * pi * fromIntegral j / fromIntegral n
  ]

blades :: Solid
blades = polar n $ rotate ex (pi / 4) $ revolution bladeAngle $ rectangle 2 0.2
  where
  n = 4
  cover = 0.5
  bladeAngle = cover * 2 * pi / fromIntegral n

mirrorZ1 :: Solid -> Solid
mirrorZ1 = _translated ez (-1) %~ mirror ez

taperedSpiral :: Solid
taperedSpiral = unitSpiral 2 0.2 $ rectangle 0.5 0.2

write :: [Solid] -> IO ()
write = writeSTEPColor (takeWhile (/= '.') __FILE__ ++ ".step") . foldr1 (stacked ex) . intersperse unitGap
