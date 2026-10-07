{-# LANGUAGE TemplateHaskell, CPP #-}
import Rapids
import Data.List

main = writeSTEPColor (takeWhile (/= '.') __FILE__ ++ ".step") $ foldr1 (stacked ex) $ intersperse unitGap
    [$red $ offset 0.1 2 [1,2] unitCube,
    $blue $ offset 0.1 0 unitCube,
    $green $ offset 0.1 [2,3,6] unitCube
    ]
