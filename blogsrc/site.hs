--------------------------------------------------------------------------------
{-# LANGUAGE OverloadedStrings #-}
import           Data.Monoid (mappend)
import           Hakyll
import Data.List
import qualified Text.Blaze.Html5                as H
import qualified Text.Blaze.Html5.Attributes as A
import Text.Blaze.Html
import System.Environment
import Control.Concurrent
import qualified Data.Map as M
import Data.Maybe (mapMaybe, listToMaybe, fromMaybe)
import Control.Monad (forM)
import Data.List.Split (splitOneOf)
import Text.Blaze.Html.Renderer.String (renderHtml)
import qualified Data.Set as S
-- ...existing code...

-- Build tags list for index.html (outer ul = tags, inner ul = first unique words)
tagListCtx :: Tags -> Context String
tagListCtx tags =
    listField "tagsList" tagCtx (mapM (makeItem . fst) (tagsMap tags))
  where
    tagCtx =
        field "tag" (return . itemBody) <>
        listFieldWith "words" wordCtx (\tagItem -> do
            let tag = itemBody tagItem
                ids = M.findWithDefault [] tag (M.fromList $ tagsMap tags)
            titlesAndRoutes <- forM ids $ \i -> do
                t <- getMetadataField i "title"
                r <- getRoute i
                return (i, t, r)

            -- Build global words from all titles across the site so words are unique across tags.
            let allIds = nub $ concat $ M.elems (M.fromList $ tagsMap tags)
            allTitlesAndRoutes <- forM allIds $ \i -> do
                t <- getMetadataField i "title"
                r <- getRoute i
                return (i, t, r)
            let allPresentTitles = mapMaybe (\(_,t,_) -> t) allTitlesAndRoutes
                globalWords = firstUniqueWords allPresentTitles

            -- For this tag, keep the global ordering but only those words that appear in the tag's titles.
            let appearsInTag w = any (\(_, mt, _) -> maybe False (\tt -> w `elem` splitOneOf " -," tt) mt) titlesAndRoutes
                wordsList = filter appearsInTag globalWords
                wordItems = map (\w ->
                    let href = case [ r | (_, Just t, Just r) <- titlesAndRoutes, w `elem` splitOneOf " -," t] of
                            x:_ -> x
                            _ -> "#"
                    in (w, href)
                  ) wordsList

            mapM makeItem wordItems
        )
    wordCtx :: Context (String, String)
    wordCtx = mconcat
      [ field "word_text" $ return . fst . itemBody
      , field "word_href" $ return . snd . itemBody
      ]

test_first_unique = firstUniqueWords ["Cubic equation solving", "Cubic spline decimation"]

firstUniqueWords :: [String] -> [String]
firstUniqueWords titles = firstUniqueOf
      (\n w -> if n == 0 then w else w ++ "-" ++ show n)
      M.empty
      (map (filter (not . null) . splitOneOf " -,") titles)


firstUniqueOf :: Ord a => (Int -> a -> b)
  -> M.Map a Int
  -> [[a]]
  -> [b]
firstUniqueOf render = go
  where
    go _ [] = []
    go seenMap (t:ts) =
      case find (`M.notMember` seenMap) t of
        Just x -> render 0 x : go (M.insert x 1 seenMap) ts
        Nothing -> case t of
          (x:_) ->
            -- one lookup alterF
            let n = M.findWithDefault 1 x seenMap
                x' = render n x
                seenMap' = M.insert x (n + 1) seenMap
            in x' : go seenMap' ts
          [] -> go seenMap ts



postCtxWithTags :: Tags -> Context String
postCtxWithTags tags = tagsFieldWith getTags linkToIndexTagAnchor (mconcat . intersperse ", ") "tags" tags `mappend` postCtx

-- | simpleRenderLink except pointing at index.html#tag_<tag>
linkToIndexTagAnchor :: String -> Maybe FilePath -> Maybe H.Html
linkToIndexTagAnchor tag _ = Just $
    H.a ! A.title (H.stringValue ("All pages tagged '"++tag++"'."))
        -- ! A.href (preEscapedStringValue $ "/#" ++ tag)
        ! A.href (preEscapedStringValue $ "/tags/" ++ tag ++ ".html")
        ! A.rel "tag"
        $ toHtml tag

--------------------------------------------------------------------------------
ghcid = do
  forkIO $ withArgs ["rebuild"] main
  withArgs ["watch"] main

main = hakyll $ do
    match "images/*" $ do
        route   idRoute
        compile copyFileCompiler

    match "css/*" $ do
        route   idRoute
        compile compressCssCompiler

    match (fromList ["about.rst", "contact.markdown"]) $ do
        route   $ setExtension "html"
        compile $ pandocCompiler
            >>= loadAndApplyTemplate "templates/default.html" defaultContext
            >>= relativizeUrls

    tags <- buildTags "posts/*" (fromCapture "tags/*.html")
    tagsRules tags $ \tag pattern -> do
      let title = "Posts tagged \"" ++ tag ++ "\""
      route idRoute
      compile $ do
          posts <- recentFirst =<< loadAll pattern
          let ctx = constField "title" title
                    `mappend` listField "posts" (postCtxWithTags tags) (return posts)
                    `mappend` defaultContext

          makeItem ""
              >>= loadAndApplyTemplate "templates/tag.html" ctx
              >>= loadAndApplyTemplate "templates/default.html" ctx
              >>= relativizeUrls
    match "posts/*" $ do
      route $ setExtension "html"
      compile $ pandocCompiler
          >>= loadAndApplyTemplate "templates/post.html"    (postCtxWithTags tags)
          >>= loadAndApplyTemplate "templates/default.html" (postCtxWithTags tags)
          >>= relativizeUrls

    create ["archive.html"] $ do
        route idRoute
        compile $ do
            posts <- recentFirst =<< loadAll "posts/*"
            let archiveCtx =
                    listField "posts" postCtx (return posts) `mappend`
                    constField "title" "Archives"            `mappend`
                    defaultContext

            makeItem ""
                >>= loadAndApplyTemplate "templates/archive.html" archiveCtx
                >>= loadAndApplyTemplate "templates/default.html" archiveCtx
                >>= relativizeUrls


    match "index.html" $ do
        route idRoute
        compile $ do
            posts <- recentFirst =<< loadAll "posts/*"
            let indexCtx =
                    tagListCtx tags <>
                    listField "posts" postCtx (return posts) <>
                    defaultContext
            getResourceBody
                >>= applyAsTemplate indexCtx
                >>= loadAndApplyTemplate "templates/default.html" indexCtx
                >>= relativizeUrls
    {-
    match "index.html" $ do
        route idRoute
        compile $ do
            posts <- recentFirst =<< loadAll "posts/*"
            let indexCtx =
                    listField "tagsList" (postCtxWithTags tags) (return []) <>
                    listField "posts" postCtx (return posts) `mappend`
                    defaultContext

            getResourceBody
                >>= applyAsTemplate indexCtx
                >>= loadAndApplyTemplate "templates/default.html" indexCtx
                >>= relativizeUrls
                -}

    match "templates/*" $ compile templateBodyCompiler


--------------------------------------------------------------------------------
postCtx :: Context String
postCtx =
    dateField "date" "%B %e, %Y" `mappend`
    defaultContext
