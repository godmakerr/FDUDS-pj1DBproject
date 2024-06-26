--
-- PostgreSQL database dump
--

-- Dumped from database version 14.2
-- Dumped by pg_dump version 14.2

SET statement_timeout = 0;
SET lock_timeout = 0;
SET idle_in_transaction_session_timeout = 0;
SET client_encoding = 'UTF8';
SET standard_conforming_strings = on;
SELECT pg_catalog.set_config('search_path', '', false);
SET check_function_bodies = false;
SET xmloption = content;
SET client_min_messages = warning;
SET row_security = off;

--
-- Name: adminpack; Type: EXTENSION; Schema: -; Owner: -
--

CREATE EXTENSION IF NOT EXISTS adminpack WITH SCHEMA pg_catalog;


--
-- Name: EXTENSION adminpack; Type: COMMENT; Schema: -; Owner: 
--

COMMENT ON EXTENSION adminpack IS 'administrative functions for PostgreSQL';


SET default_tablespace = '';

SET default_table_access_method = heap;

--
-- Name: auth_user; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.auth_user (
    id integer NOT NULL,
    username VARCHAR(255) NOT NULL,
    password VARCHAR(255) NOT NULL,
    is_superuser boolean NOT NULL,
    first_name VARCHAR(255) NOT NULL,
    last_name VARCHAR(255) NOT NULL,
    email VARCHAR(255) NOT NULL,
    is_active boolean NOT NULL
);


ALTER TABLE public.auth_user OWNER TO postgres;

--
-- Name: auth_user_id_seq; Type: SEQUENCE; Schema: public; Owner: postgres
--

CREATE SEQUENCE public.auth_user_id_seq
    AS integer
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER TABLE public.auth_user_id_seq OWNER TO postgres;

--
-- Name: auth_user_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: postgres
--

ALTER SEQUENCE public.auth_user_id_seq OWNED BY public.auth_user.id;


--
-- Name: ordered; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.ordered (
    uname VARCHAR(255) NOT NULL,
    dname VARCHAR(255) NOT NULL,
    price integer DEFAULT 0 NOT NULL,
    ocount integer DEFAULT 1,
    is_favorite boolean NOT NULL
);


ALTER TABLE public.ordered OWNER TO postgres;

--
-- Name: dish; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.dish (
    id integer NOT NULL,
    dishname VARCHAR(255) NOT NULL,
    price integer DEFAULT 0 NOT NULL,
    spiciness VARCHAR(255) NOT NULL,
    cname VARCHAR(255)
);


ALTER TABLE public.dish OWNER TO postgres;

--
-- Name: dish_id_seq; Type: SEQUENCE; Schema: public; Owner: postgres
--

CREATE SEQUENCE public.dish_id_seq
    AS integer
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER TABLE public.dish_id_seq OWNER TO postgres;

--
-- Name: dish_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: postgres
--

ALTER SEQUENCE public.dish_id_seq OWNED BY public.dish.id;


--
-- Name: cuisines; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.cuisines (
    id integer NOT NULL,
    cname VARCHAR(255) NOT NULL,
    area VARCHAR(255),
    message character varying(1000),
    sample character varying(1000)
);


ALTER TABLE public.cuisines OWNER TO postgres;

--
-- Name: cuisines_id_seq; Type: SEQUENCE; Schema: public; Owner: postgres
--

CREATE SEQUENCE public.cuisines_id_seq
    AS integer
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER TABLE public.cuisines_id_seq OWNER TO postgres;

--
-- Name: cuisines_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: postgres
--

ALTER SEQUENCE public.cuisines_id_seq OWNED BY public.cuisines.id;


--
-- Name: auth_user id; Type: DEFAULT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.auth_user ALTER COLUMN id SET DEFAULT nextval('public.auth_user_id_seq'::regclass);


--
-- Name: dish id; Type: DEFAULT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.dish ALTER COLUMN id SET DEFAULT nextval('public.dish_id_seq'::regclass);


--
-- Name: cuisines id; Type: DEFAULT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.cuisines ALTER COLUMN id SET DEFAULT nextval('public.cuisines_id_seq'::regclass);


--
-- Data for Name: auth_user; Type: TABLE DATA; Schema: public; Owner: postgres
--

COPY public.auth_user (id, username, password, is_superuser, first_name, last_name, email, is_active) FROM stdin;
1	super	R2jWSeydIBlG9X8z$f8gXThvOiMNHc6TJhn4cMtYpaH0auVYDRM3KPvuTN28=	t	super	super	super@example.com	t
\.


--
-- Data for Name: ordered; Type: TABLE DATA; Schema: public; Owner: postgres
--

COPY public.ordered (uname, dname, ocount, is_favorite) FROM stdin;
\.


--
-- Data for Name: dish; Type: TABLE DATA; Schema: public; Owner: postgres
--

COPY public.dish (id, dishname, price, spiciness, cname) FROM stdin;
1	玉记扒鸡	24	不辣	鲁菜
2	夫妻肺片	34	特辣	川菜
3	蚂蚁上树	25	微辣	川菜
4	东坡肘子	31	不辣	川菜
5	菠萝咕噜肉	28	不辣	粤菜
6	潮汕牛肉火锅	58	不辣	粤菜
7	糖粥藕	26	不辣	苏菜
8	清炖蟹粉狮子头	32	不辣	苏菜
9	真心豆腐丸	15	不辣	闽菜
10	酸辣鱿鱼汤	35	中辣	闽菜
11	舟山虾爆鳝面	27	微辣	浙菜
12	腌鲜鳜鱼	37	不辣	徽菜
13	糖油粑粑	17	不辣	湘菜
14	辣椒炒肉	27	特辣	湘菜
15	干锅牛肚	32	中辣	湘菜
16	海鲜焗饭	22	中辣	意大利菜
17	法国羊鞍扒	39	微辣	法餐
18	法式鹅肝	79	微辣	法餐
\.



--
-- Data for Name: cuisines; Type: TABLE DATA; Schema: public; Owner: postgres
--

COPY public.cuisines (id, cname, area, message, sample) FROM stdin;
0	Unknown	Unknown	\N	\N
1	鲁菜	中国山东省	鲁菜，是中国传统四大菜系（也是八大菜系）中的自发型菜系（相对于淮扬、川、粤等影响型菜系而言），中国八大菜系之首，是北方菜的代表，也是中国家常菜的基础，是历史最悠久、技法最丰富、最见功力的菜系，黄河流域烹饪文化的代表。	经典菜品有一品豆腐、糖醋鲤鱼、葱烧海参、三丝鱼翅、白扒四宝、糖醋黄河鲤鱼、九转大肠、油爆双脆、扒原壳鲍鱼、油焖大虾、醋椒鱼、糟熘鱼片、温炝鳜鱼片、芫爆鱿鱼卷、清汤银耳、木樨肉（木须肉）、胶东四大拌、糖醋里脊、红烧大虾、招远蒸丸、枣庄辣子鸡、芙蓉鸡片、氽芙蓉黄管、阳关三叠、雨前虾仁、乌云托月、黄焖鸡、锅塌黄鱼、奶汤鲫鱼、烧二冬、泰山三美汤、汆西施舌等。
2	川菜	中国四川省	川菜，作为中国四大菜系之一，以其独特的麻辣味道和丰富的烹饪技巧闻名。原料以四川地区境内的山珍、水产、蔬果为主；调辅料以川糖、花椒、辣椒及豆瓣、腐乳为主。川菜烹饪技法有煎、炒、油淋、酥炸等40种。味型以麻辣、鱼香、怪味为其突出特点	代表菜品有鱼香肉丝、宫保鸡丁、红烧牛肉、水煮肉片、夫妻肺片、麻婆豆腐、回锅肉、干煸四季豆、肉末茄子、口水鸡、香辣虾、麻辣鸡块、重庆火锅、鸡豆花、酸辣土豆丝、辣子鸡、青椒肉丝等。
3	湘菜	中国湖南省	湘菜，又叫湖南菜，是中国历史悠久的汉族八大菜系之一，早在汉朝就已经形成菜系。以湘江流域、洞庭湖区和湘西山区三种地方风味为主。湘菜制作精细，用料上比较广泛，口味多变，品种繁多；色泽上油重色浓，讲求实惠；品味上注重香辣、香鲜、软嫩；制法上以煨、炖、腊、蒸、炒诸法见称。	官府湘菜代表菜品以组庵湘菜为代表，如组庵豆腐、组庵鱼翅等；民间湘菜代表菜品有辣椒炒肉、剁椒鱼头、湘西外婆菜、吉首酸肉、牛肉粉，衡阳鱼粉，栖凤渡鱼粉，东安鸡，金鱼戏莲、永州血鸭、九嶷山兔、宁远酿豆腐、腊味合蒸、姊妹团子 、宁乡口味蛇、岳阳姜辣蛇等。
4	闽菜	中国福建省	闽菜是中国八大菜系之一，历经中原汉族文化和闽越族文化的混合而形成。发源于福州，以福州菜为基础，后又融合闽东、闽南、闽西、闽北、莆仙五地风味菜形成的菜系。狭义闽菜指以福州菜，最早起源于福建福州闽县，后来发展成福州、闽南、闽西三种流派,即广义闽菜。闽菜以口味清鲜、和醇、荤香、多汤为主，擅红糟、糖醋调味。	闽菜除招牌菜“佛跳墙”外，还有福鼎肉片、鸡汤氽海蚌、八宝红鲟饭、白炒鲜竹蛏、太极芋泥、淡糟香螺片、爆炒双脆、南煎肝、荔枝肉、醉排骨、荷包鱼翅、龙身凤尾虾、翡翠珍珠鲍、鸡茸金丝笋、肉米鱼唇、鼎边糊、福州鱼丸、肉燕、漳州卤面、五柳居、白雪鸡、长汀豆腐干等菜品和小吃，均别有风味
5	浙菜	中国浙江省	浙江菜，简称浙菜，是中国汉族八大菜系之一，其地山清水秀，物产丰富，故谚曰：“上有天堂，下有苏杭”。浙江省位于我国东海之滨，北部水道成网，素有鱼米之乡之称。西南丘陵起伏，盛产山珍野味。东部沿海渔场密布，水产资源丰富，有经济鱼类和贝壳水产品500余种，总产值居全国之首，物产丰富，佳肴自美，特色独具，有口皆碑	浙菜系主要名菜有：“西湖醋鱼”、“东坡肉”、“赛蟹羹”、“家乡南肉”、“干炸响铃”、“荷叶粉蒸肉”、“西湖莼菜汤”、“龙井虾仁”、“杭州煨鸡”、“虎跑素火煺”、“干菜焖肉”、“蛤蜊黄鱼羹”、“叫化童鸡”“香酥焖肉”、“丝瓜卤蒸黄鱼”、“三丝拌蛏”、“油焖春笋”、“虾爆鳝背”等数百种。
6	徽菜	中国安徽省	徽菜即为安徽菜，根据《徽菜标准化体系表》，将传统徽菜的范围扩大了，徽菜正式以皖南菜为代表的皖南菜、皖江菜、合肥菜、淮南菜、皖北菜的总称。其中皖南风味以徽州地方菜肴为代表，它是徽菜的主流和渊源。以徽州特产为主要原料，在采用民间传统烹调技法的基础上，吸收其它菜系技艺之长而烹制的以咸鲜味为主的地方菜肴。	徽菜历史上有五六百个品种，经过挑选巩固和创新，如今确定的有3000多个新老品种。其最有代表性的菜肴有：腌鲜鳜鱼（臭鳜鱼）、问政山笋、徽州毛豆腐、徽州蒸鸡、胡氏一品锅、无为熏鸭、毛峰熏鲥鱼、炒虾丝、符离集烧鸡、李鸿章大杂烩、三河酥鸭、包公鱼、吴王贡鹅、荠菜圆子、淮王鱼炖豆腐等名菜佳肴。
7	苏菜	中国江苏省	江苏菜，中国汉族八大菜系之一，简称苏菜。由于苏菜和浙菜相近，因此和浙菜统称江浙菜系。主要以金陵菜、淮扬菜、苏锡菜、徐海菜等地方菜组成。江苏菜起源于二千多年前，其中金陵菜起源于先秦时期，当时吴人善制炙鱼、蒸鱼和鱼片，一千多年前，鸭已为南京美食。南宋时，苏菜和浙菜同为“南食”的两大台柱	其名菜有金陵烤鸭、彭城鱼丸、老鸭汤、炖生敲、烤方、羊方藏鱼、水晶肴蹄、清炖蟹粉狮子头、霸王别姬、黄泥煨鸡、清炖鸡孚、盐水鸭、金陵板鸭、金香饼、鸡汤煮干丝、肉酿生麸、红烧沙光鱼、凤尾虾、三套鸭、无锡肉骨头、陆稿荐酱猪头肉等。
8	粤菜	中国广东省	粤菜即广东菜，是中国四大菜系、八大菜系之一。粤菜包括广府菜（即广州府菜）、潮州菜（又称潮汕菜）、东江菜（又称客家菜）、粤西菜。主打粤菜的澳门、顺德和潮州三市是联合国评定的“世界美食之都”	著名的广府菜有：白切鸡、烧鹅、烤乳猪、红烧乳鸽、蜜汁叉烧、脆皮烧肉、上汤焗龙虾、清蒸东星斑、阿一鲍鱼、鲍汁扣辽参、白灼象拔蚌、椒盐濑尿虾、香煎芙蓉蛋、鼎湖上素、烟筒白菜、鱼香茄子煲、太爷鸡、赛螃蟹、香芋扣肉、南乳粗斋煲、菜胆炖鱼翅、麒麟鲈鱼、碗仔翅、流沙包、猪脚姜、糯米鸡、钵仔糕等
9	意大利菜	意大利	意大利菜非常丰富，菜品成千上万。源远流长的意大利餐，对欧美国家的餐饮产生了深厚影响，并发展出包括法餐、美国餐在内的多种派系，故有“西餐之母”之美称。	意大利菜有佛罗伦萨牛排、罗马魔鬼鸡、那不勒斯烤龙虾、巴里甲鱼、奥斯勃克牛肘肉、扎马格龙沙拉，米列斯特通心粉、鸡蛋肉末沙司、板肉白豆沙拉子、青椒焖鸡、烩大虾、烤鱼、冷鸡、白豆汤、火腿切面条等名食
10	法餐	法国	法国菜是世界上著名菜系之一，已为众所公认。它的口感之细腻、酱料之美味、餐具摆设之华美，简直可称之为一种艺术。法国的烹饪技术一向著称于世界。法国菜不仅美味可口，而且菜肴的种类很多，烹调方法也有独到之处。	法国菜中一些经典名菜：洋葱汤、香煎鹅肝、鸭腿、法式奶油焗鳕鱼、鸡肉卷、海鲜浓汤、法式焗饭、巴黎甜品、克莱尔、法式焦糖布丁
\.


--
-- Name: auth_user_id_seq; Type: SEQUENCE SET; Schema: public; Owner: postgres
--

SELECT pg_catalog.setval('public.auth_user_id_seq', 2, true);


--
-- Name: dish_id_seq; Type: SEQUENCE SET; Schema: public; Owner: postgres
--

SELECT pg_catalog.setval('public.dish_id_seq', 18, true);


--
-- Name: cuisines_id_seq; Type: SEQUENCE SET; Schema: public; Owner: postgres
--

SELECT pg_catalog.setval('public.cuisines_id_seq', 10, true);


--
-- Name: auth_user auth_user_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.auth_user
    ADD CONSTRAINT auth_user_pkey PRIMARY KEY (id);


--
-- Name: auth_user auth_user_username_key; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.auth_user
    ADD CONSTRAINT auth_user_username_key UNIQUE (username);


--
-- Name: ordered ordered_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.ordered
    ADD CONSTRAINT ordered_pkey PRIMARY KEY (uname, dname);


--
-- Name: dish dish_dishname_key; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.dish
    ADD CONSTRAINT dish_dishname_key UNIQUE (dishname);


--
-- Name: dish dish_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.dish
    ADD CONSTRAINT dish_pkey PRIMARY KEY (id);


--
-- Name: cuisines cuisines_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.cuisines
    ADD CONSTRAINT cuisines_pkey PRIMARY KEY (id);


--
-- Name: ordered ordered_dname_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.ordered
    ADD CONSTRAINT ordered_dname_fkey FOREIGN KEY (dname) REFERENCES public.dish(dishname);


--
-- Name: ordered ordered_uname_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.ordered
    ADD CONSTRAINT ordered_uname_fkey FOREIGN KEY (uname) REFERENCES public.auth_user(username);


--
-- PostgreSQL database dump complete
--

