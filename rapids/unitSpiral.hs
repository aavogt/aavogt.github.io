{-# LANGUAGE CPP, TemplateHaskell #-}
import Data.List
import Rapids

main = do
  writeSTEPColor (takeWhile (/= '.') __FILE__ ++ ".step") $
    foldr1 (stacked ex) $
      intersperse
        unitGap
        [ $green $ unitSpiral 2 $ rectangle 0.5 0.2,
          $red $ offset 0.05 [5, 6] $ mirrorZ1 taperedSpiral,
          $blue fan
        ]

fan =
  unions
    [ rotate ez th ex (pi / 4) $ revolution bladeAngle $ rectangle 2 0.2
    | j <- [0 .. n - 1],
      let th = 2 * pi * fromIntegral j / fromIntegral n
    ]
    + scale 0.5 0.3 unitCylinder
    - scale 0.3 1 unitCylinder
  where
    n = 4
    cover = 0.5
    bladeAngle = cover * 2 * pi / fromIntegral n

mirrorZ1 = _translated ez (-1) %~ mirror ez

taperedSpiral = unitSpiral 2 0.2 $ rectangle 0.5 0.2
