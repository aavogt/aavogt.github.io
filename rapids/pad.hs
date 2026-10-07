{-# LANGUAGE TemplateHaskell, CPP #-}
import Rapids
import Data.List

main = writeSTEPColor (takeWhile (/= '.') __FILE__ ++ ".step") $ foldr1 (stacked ex) $ intersperse unitGap
    [$red $ pad 2 unitCircle,
    $blue $ pad 2 0.1 unitCircle,
    $green $ pad 2 3 4 $ unitPolygon 5,
    $yellow $ pad (V3 2 3 4) $ circle 0.5
    ]
