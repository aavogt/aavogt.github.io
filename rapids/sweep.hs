{-# LANGUAGE TemplateHaskell, QuasiQuotes, CPP #-}
import Rapids
import Rapids.SVG
import Data.List

main = writeSTEPColor (takeWhile (/= '.') __FILE__ ++ ".step") $ foldr1 (stacked ex) $ intersperse unitGap
  [$red $ offset 0.2 0 [1,4] $ sweep curve unitCircle,
  $blue $ offset 0.2 2 [1,4] $ sweep curve (uScale2D 0.5 unitCircle)
  ]

curve = foldMap toPath [svg| c 0,1,1,1,2,2 |]
