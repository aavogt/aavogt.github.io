{-# LANGUAGE TemplateHaskell, CPP #-}
import Rapids
import Data.List

r = $red (torus 5 2) 
b = translate ex 5 $ rotateDeg ex 60 $ $blue $ torus 5 2

main = writeSTEPColor (takeWhile (/= '.') __FILE__ ++ ".step") $ foldr1 (stacked ex) $ intersperse unitGap
    [r + b,
    section (r + b) & pad 0.1, -- colors wrong
    silhouette (r + b) & pad 0.1 & $green -- wrong geometry no colors
    ]
