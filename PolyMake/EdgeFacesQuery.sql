select * from MEFaces
where LWFlag = 'P';

select tlid from MEEdgeFaces
where tfidl IN  (Select tfid from MEFaces where LWFlag = 'P') AND
tfidr IN  (Select tfid from MEFaces where LWFlag = 'L');

select tlid, tfidl, F1.LWFlag, tfidr, F2.LWFlag
from MEEdgeFaces EF
JOIN MEFaces F1 ON tfidl = F1.tfid
JOIN MEFaces F2 ON tfidr = F2.tfid
WHERE F1.LWFlag = 'P' AND F2.LWFlag = 'L';

select * from MEEdgeFaces where tlid in (75630408 );

where tfidl IN  (Select tfid from MEFaces where LWFlag = 'P') AND
tfidr IN  (Select tfid from MEFaces where LWFlag = 'L');

select tlid, tfidl, F1.LWFlag, tfidr, F2.LWFlag from MEEdgeFaces EF
JOIN MEFaces F1 ON tfidl = F1.tfid
LEFT OUTER JOIN MEFaces F2 ON tfidr = F2.tfid
WHERE F1.LWFlag = 'L' AND F2.LWFlag = 'P';

select tlid from MEEdgeFaces
where tfidl IN  (Select tfid from MEFaces where LWFlag = 'L') AND
tfidr IN  (Select tfid from MEFaces where LWFlag = 'P');

select  tlid from MEEdgeFaces
where tfidl IN  (Select tfid from MEFaces where LWFlag = 'P') AND
tfidr IN  (Select tfid from MEFaces where LWFlag = 'P');

select * from MEEdgeFaces
where tlid = 75634817   ;
select * from MEEdgeFaces t1, MEFaces t2
where (t1.tfidl = t2.tfid OR t1.tfidr = t2.tfid)
AND tlid = 75634817 ;


select * from MEFaces where tfid IN (202562549,202562524);

select tlid, tfid, LWFlag
FROM MEEdgeFaces
LEFT OUTER JOIN MEFaces ON tfidl = tfid
WHERE LWFlag = 'L'
UNION
select tlid, tfid, LWFlag
FROM MEEdgeFaces
FULL OUTER JOIN MEFaces ON tfidr = tfid
WHERE LWFlag = 'L'
order by tlid;

select tlid, tfid, LWFlag
FROM MEEdgeFaces
LEFT OUTER JOIN MEFaces ON tfidr = tfid
and LWFlag = 'P';

select tlid, tfid, LWFlag
FROM MEEdgeFaces
LEFT OUTER JOIN MEFaces ON tfidl = tfid;

select tlid, tfid, LWFlag
FROM MEEdgeFaces
FULL OUTER JOIN MEFaces ON tfidr = tfid;

WHERE LWFlag = 'P';



, tfidr, F2.LWFlag from MEEdgeFaces EF
OUTER 
where tfidl IN  (Select tfid from MEFaces where LWFlag = 'L') AND
tfidr IN  (Select tfid from MEFaces where LWFlag = 'P');

select tfid from MEFaces
where LWFlag = 'P';

select tlid, tfidl, tfidr
from MEedgeFaces
LEFT OUTER JOIN MEFaces F1 ON tfidl = F1.tfid
LEFT OUTER JOIN MEFaces F2 ON tfidr = F2.tfid;

select tlid, tfidl, tfidr, F1.tfid, F1.LWFlag, F2.tfid, F2.LWFlag
from MEedgeFaces
LEFT OUTER JOIN MEFaces F1 ON tfidl = F1.tfid AND F1.LWFlag = 'L'
JOIN MEFaces F2 ON tfidr = F2.tfid AND F2.LWFlag = 'P'
WHERE tlid in (614502568); 

select tlid, /*tfidl, tfidr,*/ F1.tfid, F1.LWFlag, F2.tfid, F2.LWFlag
from MEedgeFaces
LEFT OUTER JOIN MEFaces F1 ON tfidl = F1.tfid
JOIN MEFaces F2 ON tfidr = F2.tfid 
WHERE (F1.LWFlag = 'L' OR F1.LWFLAG IS NULL) AND F2.LWFlag = 'P' 
ORDER BY tlid;
AND tlid in (614502568); 

select tlid, /*tfidl, tfidr,*/ F1.tfid, F1.LWFlag, F2.tfid, F2.LWFlag
from MEedgeFaces
JOIN MEFaces F1 ON tfidl = F1.tfid
LEFT OUTER JOIN MEFaces F2 ON tfidr = F2.tfid 
WHERE (F2.LWFlag = 'L' OR F2.LWFLAG IS NULL) AND F1.LWFlag = 'P' 
ORDER BY tlid;

select tlid, /*tfidl, tfidr,*/ F1.tfid, F1.LWFlag, F2.tfid, F2.LWFlag
from MEedgeFaces, MEFaces F1, MEFaces F2
WHERE tfidl = F1.tfid AND tfidr = F2.tfid
AND (F1.LWFlag = 'L' OR F1.LWFLAG IS NULL) AND F2.LWFlag = 'P' ;