test:�����ԣ�
1.memtest.c �ڴ���
2.redis_benchmark.c ����redis���ܲ��Ե�ʵ�֡�
3.redis_check_aof.c ���ڸ�����־����ʵ�֡�
4.redis_check_dump.c ���ڱ������ݿ����ʵ�֡�
5.testhelp.c һ��C����С�Ͳ��Կ�ܡ�

struct:���ṹ�壩
1.adlist.c ���ڶ�list�Ķ��壬���Ǹ�˫������ṹ
2.dict.c ��Ҫ�����ڴ��е�hash���й���
3.sds.c ���ڶ��ַ����Ķ���
4.sparkline.c һ��ӵ��sample�б������
5.t_hash.c hash��Server/Client�е�Ӧ���������Ҫͨ��redisObject��������ת����
6.t_list.c list��Server/Client�е�Ӧ���������Ҫͨ��redisObject��������ת����
7.t_set.c  set��Server/Client�е�Ӧ���������Ҫͨ��redisObject��������ת����
8.t_string.c string��Server/Client�е�Ӧ���������Ҫͨ��redisObject��������ת����
9.t_zset.c zset��Server/Client�е�Ӧ���������Ҫͨ��redisObject��������ת����
10.ziplist.c  ziplist��һ��������list�Ĵ洢��������ԭ��������zipmap��
11.zipmap.c  zipmap��һ��������hash�Ĵ洢����

data:�����ݲ�����
1.aof.c ȫ��Ϊappend only file�����þ��Ǽ�¼ÿ�ε�д����,�������ϵ������ʱ�����������ָ����ݿ�״̬��
2.config.c ���ڽ������ļ�redis.conf�ļ��е����ö�ȡ����������ͨ������ŵ�server�����С�
3.db.c����Redis�ڴ����ݿ����ز�����
4.multi.c���������������
5.rdb.c  ����Redis�������ݿ����ز�����Ĭ���ļ���dump.rdb��ͨ�������ļ���ã��������Ĳ����������棬�Ƴ�����ѯ�ȵȡ�
6.replication.c �����������ݿ�ĸ��Ʋ�����ʵ�֡�

tool:�����ߣ�
1.bitops.c λ���������
2.debug.c ���ڵ���ʱʹ��
3.endianconv.c �ߵ�λת������ͬϵͳ���ߵ�λ˳��ͬ
4.help.h  �������������ʾ��Ϣ
5.lzf_c.c ѹ���㷨ϵ��
6.lzf_d.c  ѹ���㷨ϵ��
7.rand.c ���ڲ��������
8.release.c ���ڷ���ʱʹ��
9.sha1.c sha�����㷨��ʵ��
10.util.c  ͨ�ù��߷���
11.crc64.c ѭ������У��

event:���¼���
1.ae.c ����Redis���¼�������������¼��ͳ�ʱ�¼���
2.ae_epoll.c ʵ����epollϵͳ���õĽӿ�
3.ae_evport.c ʵ����evportϵͳ���õĽӿ�
4.ae_kqueue.c ʵ����kqueuexϵͳ���õĽӿ�
5.ae_select.c ʵ����selectϵͳ���õĽӿ�

baseinfo:��������Ϣ��
1.asciilogo,c redis��logo��ʾ
2.version.h����Redis�İ汾��

compatible:�����ݣ�
1.fmacros.h ����Macϵͳ�µ�����
2.solarisfixes.h ����solary�µ�����

main:��������
1.redis.c redis����˳���
2.redis_cli.c redis�ͻ��˳���

net:�����磩
1.anet.c ��ΪServer/Clientͨ�ŵĻ�����װ
2.networking.c ����Э�鴫�䷽��������صĶ���������ļ������ˡ�

wrapper:����װ�ࣩ
1.bio.c background I/O����˼��������̨�߳��õ�
2.hyperloglog.c һ�����ڴ����ݴ���ģ�����ͳ���㷨
3.intset.c  ������Χ�ڵ�ʹ��set�����������set������
4.latency.c �ӳ���
5.migrate.c ����Ǩ���࣬��������Ļ�ԭǨ�Ƶ�
6.notify.c ֪ͨ��
7.object.c  ���ڴ������ͷ�redisObject����
8.pqsort.c  �����㷨��
9.pubsub.c ���ڶ���ģʽ��ʵ�֣��е�������Client�㲥���͵ķ�ʽ��
10.rio.c redis�����һ��I/O��
11.slowlog.c һ����־���͵ģ���¼��ʱ��ѯ��¼
12.sort.c �����㷨�࣬��pqsort.cʹ�õĳ�����ͬ
13.syncio.c ����ͬ��Socket���ļ�I/O������
14.zmalloc.c ����Redis���ڴ����ķ�װʵ��

others:�������һЩ����ʱ�����Ǻ��������,����û�н����ˣ�
1.scripting.c
2.sentinel.c
2.setproctitle.c
3.valgrind.sh
4.redisassert.h

Redis�е�11���������:
1.hyperloglog����ͳ���㷨��ʵ��(http://blog.csdn.net/androidlushangderen/article/details/40683763)
2.zmalloc�ڴ���������ʵ��(http://blog.csdn.net/androidlushangderen/article/details/40659331)
3.multi�������(http://blog.csdn.net/androidlushangderen/article/details/40392209)
4.redis-benchmark���ܲ���(http://blog.csdn.net/androidlushangderen/article/details/40211907)
5.zipmapѹ���ṹ�����(http://blog.csdn.net/androidlushangderen/article/details/39994599)
6.sparkline΢��ͼ���������(http://blog.csdn.net/androidlushangderen/article/details/39964591)
7.�������ü���ʵ���ڴ����(http://blog.csdn.net/androidlushangderen/article/details/40716469)
8.fork�ӽ���ʵ�ֺ�̨����(http://blog.csdn.net/androidlushangderen/article/details/40266579)
9.long long ����תΪString���ͷ���(http://blog.csdn.net/androidlushangderen/article/details/40649623)
10.������ʽ��ʵ���㷨(http://blog.csdn.net/androidlushangderen/article/details/40649623)
11.Redis��drand48()����㷨��ʵ��(http://blog.csdn.net/androidlushangderen/article/details/40582189)
