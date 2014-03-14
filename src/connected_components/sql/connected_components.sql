--
-- Copyright (c) 2014 Adrien ANDRÉ
--


-----------------------------------------------------------------------
-- Core function for connected components computation
-----------------------------------------------------------------------
CREATE OR REPLACE FUNCTION pgr_connected_components(sql text)
        RETURNS TABLE (INTEGER vertex, INTEGER component)
        AS '$libdir/librouting', 'connected_components'
        LANGUAGE c IMMUTABLE STRICT;

